#include "CaveInGameMode.h"
#include "CaveIn/Actors/CaveTile.h"
#include "CaveIn/Characters/BaseCharacter.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void ACaveInGameMode::BeginPlay() 
{
    if (UGameplayStatics::GetCurrentLevelName(GetWorld()) == "Level1") 
    {
        NumFilledTiles = 0;
        PlayerWin = false;
        IsGameOver = false;
        MiddleToEndFilled = false;
        // Initialize Tiles with nullptrs
        Tiles.Init(nullptr, NumTilesY * NumTilesX);
        // Spawn our final block
        FTransform Transform;
        Transform.SetLocation(FinalBlockLocation);
        Transform.SetRotation(FRotator::ZeroRotator.Quaternion());
        Transform.SetScale3D(FVector(2, 2, 2));
        FinalBlockRef = GetWorld()->SpawnActorDeferred<ACaveTile>(TileClass, Transform);
        FinalBlockRef->SetIndestructible(false);
        FinalBlockRef->SetIsFinalBlock(true);
        FinalBlockRef->FinishSpawning(Transform);
        // Get a reference to our player character
        PlayerRef = Cast<ABaseCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
        // Start our tile generation
        GetWorldTimerManager().SetTimer(TileTimerHandle, this, &ACaveInGameMode::StartShake, 3, false);
    }
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
        if (SpawnTile(X, Y))
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
                UE_LOG(LogTemp, Error, TEXT("Brute Force Failed!"));
            }
        }
        // Make sure we log when this happens for bookkeeping
        if (SafetyCounter == 0) 
        {
            UE_LOG(LogTemp, Error, TEXT("Hit the Safety Counter!"));
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
        UE_LOG(LogTemp, Warning, TEXT("Brute Forcing, Middle->End..."));
        for (size_t Index = (Tiles.Num()/2); Index < Tiles.Num(); Index++) 
        {
            // Find the first empty spot
            if (Tiles[Index] == nullptr) {
                // Determine X and Y values
                int32 X = Index % NumTilesX;
                int32 Y = FMath::DivideAndRoundDown<int32>(Index, NumTilesX);
                UE_LOG(LogTemp, Warning, TEXT("Successful!"));
                // Attempt to spawn, return results
                return SpawnTile(X, Y);
            }
        }
        MiddleToEndFilled = true;
    }
    // Loop through the Tiles array from the middle->beginning
    UE_LOG(LogTemp, Warning, TEXT("Brute Forcing, Middle->Start..."));
    for (size_t Index = (Tiles.Num()/2); Index >= 0; Index--) 
    {
        // Find the first empty spot
        if (Tiles[Index] == nullptr) {
            // Determine X and Y values
            int32 X = Index % NumTilesX;
            int32 Y = FMath::DivideAndRoundDown<int32>(Index, NumTilesX);
            UE_LOG(LogTemp, Warning, TEXT("Successful!"));
            // Attempt to spawn, return results
            return SpawnTile(X, Y);
        }
    }
    return false;
}

// Attempt to spawn a tile at a given X and Y location
bool ACaveInGameMode::SpawnTile(int32 X, int32 Y) 
{
    int32 Index = GetIndex(X, Y);
    // Randomly make certain tiles indestructible
    bool Indestructible = false;
    bool IsBomb = false;
    if (Index != 4 && Index != 5) 
    {
        int32 RandomNumber = FMath::RandRange(1, 100);
        if (RandomNumber <= 20) { Indestructible = true; }
        if (RandomNumber >= 80) { IsBomb = true; }
    }
    // Actually create the tile and update Tiles array
    if (Tiles[Index] == nullptr)
    {
        FVector Location = FVector(X * 200, Y * 200, SpawnHeight);
        FTransform Transform;
        Transform.SetLocation(Location);
        Transform.SetRotation(FRotator::ZeroRotator.Quaternion());
        ACaveTile* NewTile = GetWorld()->SpawnActorDeferred<ACaveTile>(TileClass, Transform);
        NewTile->SetIndestructible(Indestructible);
        NewTile->SetIsBombBlock(IsBomb);
        NewTile->FinishSpawning(Transform);
        Tiles[Index] = NewTile;
        NumFilledTiles++;
        return true;
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
            DestroyedTile->HandleDestruction();
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
            // Call tile-specific destruction logic
            DestroyedTile->HandleDestruction();
        }
    }
}

void ACaveInGameMode::HandleGameOver(bool PlayerWon) 
{
    if (!IsGameOver) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Game Over!"));
        // Clear timers
        GetWorldTimerManager().ClearTimer(TileTimerHandle);
        // Mark us as having won or lost
        IsGameOver = true;
        PlayerWin = PlayerWon;
        // Call Blueprints to show widgets accordingly
        GameOver(PlayerWin);
    }
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