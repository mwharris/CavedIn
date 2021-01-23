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
    NumFilledTiles = 0;
    PlayerWin = false;
    IsGameOver = false;
    MiddleToEndFilled = false;
    // Initialize Tiles with nullptrs
    Tiles.Init(nullptr, NumTilesY * NumTilesX);
    // Get a reference to our player character
    PlayerRef = Cast<ABaseCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    // Spawn our final block with a delay
    GetWorldTimerManager().SetTimer(ExitSpawnTimerHandle, this, &ACaveInGameMode::SpawnFinalBlock, FinalBlockSpawnDelay, false);
}

// TODO: Fix this up to use GetWorld()->SpawnActor<>() properly
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

/**
 *  IMPROVEMENTS:
 *  1. On a Tile Collision, loop forward from there to place the tile
 *  2. If that doesn't work then loop backwards
 *  3. If both don't work, brute force
 */
void ACaveInGameMode::TileSpawnTick() 
{
    // Kill our earthquake audio
    if (EarthquakeAudio)
    {
        EarthquakeAudio->SetActive(false);
    }
    // Determine a random amount of tiles to generate between Min and Max settings
    int32 EmptyTiles = Tiles.Num() - NumFilledTiles;
    int32 TilesToGenerate = FMath::RandRange(MinTilesPerShake, MaxTilesPerShake);
    if (EmptyTiles < TilesToGenerate) 
    {
        TilesToGenerate = EmptyTiles;
    }
    // Loop until we generate enough tiles
    int32 SafetyCounter = 100;
    while (SafetyCounter > 0 && TilesToGenerate > 0)
    {
        // Pick a random index and attempt to spawn a tile
        int32 X = FMath::RandRange(0, NumTilesX - 1);
        int32 Y = FMath::RandRange(0, NumTilesY - 1);
        // Attempt to spawn this tile at a random location
        if (SpawnTile(X, Y, false))
        {
            TilesToGenerate--;
        }
        // Index was filled - brute force it
        else 
        {
            if(BruteForceSpawn())
            {
                TilesToGenerate--;
            }
            else
            {
                SafetyCounter--;
            }
        }
    }
    // Check if the player got Caved In
    if (NumFilledTiles == Tiles.Num()) 
    {
        HandleGameOver(false);
    }
    // Restart our shake timer
    else 
    {
        GetWorldTimerManager().SetTimer(TileTimerHandle, this, &ACaveInGameMode::StartShake, Downtime, false);
    }
}

bool ACaveInGameMode::BruteForceSpawn() 
{
    // Loop through the Tiles array from the middle->end
    if (!MiddleToEndFilled)
    {
        for (size_t Index = (Tiles.Num()/2); Index < Tiles.Num(); Index++) 
        {
            // Find the first empty spot
            if (Tiles[Index] == nullptr) {
                // Determine X and Y values
                int32 X = Index % NumTilesX;
                int32 Y = FMath::DivideAndRoundDown<int32>(Index, NumTilesX);
                // Attempt to spawn, return results
                return SpawnTile(X, Y, true);
            }
        }
        MiddleToEndFilled = true;
    }
    // Loop through the Tiles array from the middle->beginning
    for (size_t Index = (Tiles.Num()/2); Index >= 0; Index--) 
    {
        // Find the first empty spot
        if (Tiles[Index] == nullptr) {
            // Determine X and Y values
            int32 X = Index % NumTilesX;
            int32 Y = FMath::DivideAndRoundDown<int32>(Index, NumTilesX);
            // Attempt to spawn, return results
            return SpawnTile(X, Y, true);
        }
    }
    return false;
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
            if (Indestructible && IsBomb) 
            {
                UE_LOG(LogTemp, Error, TEXT("Created an indestructible bomb!"));
            }
            NewTile->SetActorLocation(FVector(X * 200, Y * 200, SpawnHeight));
            NewTile->SetActorRotation(FRotator::ZeroRotator);
            NewTile->SetActorEnableCollision(true);
            NewTile->SetActive(true);
            NewTile->SetIndestructible(Indestructible);
            NewTile->SetIsBombBlock(IsBomb);
            NewTile->SetIsFinalBlock(false);
            NewTile->StartFalling();
            Tiles[Index] = NewTile;
            NumFilledTiles++;
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
            NumFilledTiles--;
            if (Index >= (Tiles.Num()/2)) 
            {
                MiddleToEndFilled = false;
            }
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