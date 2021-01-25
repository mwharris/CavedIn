#include "CaveInGameMode.h"
#include "CaveIn/Actors/CaveTile.h"
#include "CaveIn/Actors/PooledActor.h"
#include "CaveIn/ActorComponents/ObjectPool.h"
#include "CaveIn/Characters/BaseCharacter.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACaveInGameMode::ACaveInGameMode() 
{
    ObjectPooler = CreateDefaultSubobject<UObjectPool>(TEXT("Object Pooler"));
}

void ACaveInGameMode::BeginPlay() 
{
    Super::BeginPlay();
    PlayerWin = false;
    IsGameOver = false;
    // Initialize Tiles with nullptrs
    int32 ArraySize = NumTilesY * NumTilesX;
    Tiles.Init(nullptr, ArraySize);
    for (int32 i = 0; i < ArraySize; i++) 
    {
        int32 X = i % NumTilesX;
        int32 Y = FMath::DivideAndRoundDown<int32>(i, NumTilesX);
        EmptyTiles.Add(FVector2D(X, Y));
    }
    // Get a reference to our player character
    PlayerRef = Cast<ABaseCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    // Spawn our final block with a delay
    GetWorldTimerManager().SetTimer(ExitSpawnTimerHandle, this, &ACaveInGameMode::SpawnFinalBlock, FinalBlockSpawnDelay, false);
}

void ACaveInGameMode::SpawnFinalBlock() 
{
    FActorSpawnParameters params;
    FinalBlockRef = GetWorld()->SpawnActor<ACaveTile>(TileClass, FinalBlockLocation, FRotator::ZeroRotator, params);
    FinalBlockRef->SetIndestructible(false);
    FinalBlockRef->SetIsFinalBlock(true);
    FinalBlockRef->SetActorScale3D(FVector(2, 2, 2));
    FinalBlockRef->StartFalling();
    // Start our tile generation
    GetWorldTimerManager().SetTimer(TileTimerHandle, this, &ACaveInGameMode::StartShake, 3, false);
}

void ACaveInGameMode::StartShake() 
{
    EarthquakeAudio = UGameplayStatics::SpawnSoundAtLocation(this, EarthquakeSound, FVector::ZeroVector, FRotator::ZeroRotator);
    GetWorld()->GetFirstPlayerController()->ClientPlayCameraShake(WarningShake);
    GetWorldTimerManager().SetTimer(TileTimerHandle, this, &ACaveInGameMode::TileSpawnTick, ShakeTime, false);
}

void ACaveInGameMode::TileSpawnTick() 
{
    // Kill our earthquake audio
    if (EarthquakeAudio)
    {
        EarthquakeAudio->SetActive(false);
    }
    // Determine a random amount of tiles to generate between Min and Max settings
    int32 TilesToGenerate = FMath::RandRange(MinTilesPerShake, MaxTilesPerShake);
    if (EmptyTiles.Num() < TilesToGenerate) 
    {
        TilesToGenerate = EmptyTiles.Num();
    }
    // Loop until we generate enough tiles
    int32 SafetyCounter = 100;
    while (SafetyCounter > 0 && TilesToGenerate > 0)
    {
        // Pick a random X and Y position from our array of empty indexes 
        int32 RandIndex = FMath::RandRange(0, (EmptyTiles.Num()-1));
        FVector2D XAndYPos = EmptyTiles[RandIndex];
        // Attempt to spawn this tile at a random location
        if (SpawnTile(XAndYPos.X, XAndYPos.Y, false))
        {
            EmptyTiles.RemoveAt(RandIndex);
            TilesToGenerate--;
        }
        // Index was filled - brute force it
        else 
        {
            UE_LOG(LogTemp, Error, TEXT("Attempted to Brute Force!"));
            SafetyCounter--;
        }
    }
    // Check if the player got Caved In
    if (EmptyTiles.Num() == 0) 
    {
        HandleGameOver(false);
    }
    // Restart our shake timer
    else 
    {
        GetWorldTimerManager().SetTimer(TileTimerHandle, this, &ACaveInGameMode::StartShake, Downtime, false);
    }
}

// Attempt to spawn a tile at a given X and Y location
bool ACaveInGameMode::SpawnTile(int32 X, int32 Y, bool BruteForceFlag) 
{
    // Convert X and Y position to array index
    int32 Index = GetIndex(X, Y);
    // Determine if this tile should be indestructible, a bomb, or normal
    bool Indestructible = false;
    bool IsBomb = false;
    if (Index != 4 && Index != 5) 
    {
        int32 RandomNumber = FMath::RandRange(1, 100);
        if ((BruteForceFlag && RandomNumber <= 5) || (!BruteForceFlag && RandomNumber <= 15)) { Indestructible = true; }
        if (RandomNumber >= 80) { IsBomb = true; }
    }
    // Actually create the tile and update Tiles array
    if (Tiles[Index] == nullptr)
    {
        APooledActor* PooledActor = ObjectPooler->GetPooledObject();
        if (PooledActor == nullptr) 
        {
            UE_LOG(LogTemp, Error, TEXT("Cannot spawn tile!"));
            return false;
        }
        else if (ACaveTile* NewTile = Cast<ACaveTile>(PooledActor))
        {
            NewTile->InitTile(FVector(X * 200, Y * 200, SpawnHeight), FRotator::ZeroRotator, Indestructible, IsBomb, false);
            Tiles[Index] = NewTile;
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Could not cast PooledActor to CaveTile!"));
            return false;
        }
    }
    return false;
}

void ACaveInGameMode::ActorDied(AActor* DeadActor) 
{
    // Check if it's a Tile that was destroyed
    if (ACaveTile* DestroyedTile = Cast<ACaveTile>(DeadActor)) 
    {
        if (DestroyedTile->GetIsFinalBlock()) 
        {
            FinalBlockRef = nullptr;
            DestroyedTile->Reset();
        }
        else
        {
            // Null out this tile's index in the array
            FVector Location = DestroyedTile->GetActorLocation();
            int32 X = Location.X / 200;
            int32 Y = Location.Y / 200;
            int32 Index = GetIndex(X, Y);
            Tiles[Index] = nullptr;
            // Update other bookeeping variables
            EmptyTiles.Add(FVector2D(X, Y));
            //Deal damage to the door if we destroyed a bomb
            if (DestroyedTile && DestroyedTile->GetIsBombBlock() && ExplosionSound && PlayerRef) 
            {  
                UGameplayStatics::SpawnSoundAtLocation(this, ExplosionSound, DestroyedTile->GetActorLocation());
                PlayerRef->ApplyDamageToFinalBlock(FinalBlockRef);
            }
            // Reset the Tile's flags
            DestroyedTile->Reset();
        }
    }
}

void ACaveInGameMode::HandleGameOver(bool PlayerWon) 
{
    if (IsGameOver) { return; }
    // Clear timers
    GetWorldTimerManager().ClearTimer(TileTimerHandle);
    // Mark us as having won or lost
    IsGameOver = true;
    PlayerWin = PlayerWon;
    // Call Blueprints to show widgets accordingly
    GameOver(PlayerWin);
}

int32 ACaveInGameMode::GetIndex(int32 X, int32 Y) const
{
    return NumTilesX * Y + X;
}

bool ACaveInGameMode::DidPlayerWin() const
{
    return PlayerWin;
}

bool ACaveInGameMode::GetIsGameOver() const
{
    return IsGameOver;
}