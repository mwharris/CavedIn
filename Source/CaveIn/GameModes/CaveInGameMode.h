#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CaveInGameMode.generated.h"

class ACaveTile;
class ABaseCharacter;
class UAudioComponent;

UCLASS()
class CAVEIN_API ACaveInGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tiling")
	float SpawnHeight = 1000;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool DidPlayerWin() const;
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetIsGameOver() const;

	void ActorDied(AActor* DeadActor);
	void HandleGameOver(bool PlayerWon);
	void StartGame();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void GameOver(bool PlayerWon);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tiling", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ACaveTile> TileClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tiling", meta = (AllowPrivateAccess = "true"))
	int32 NumTilesX = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tiling", meta = (AllowPrivateAccess = "true"))
	int32 NumTilesY = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Final", meta = (AllowPrivateAccess = "true"))
	FVector FinalBlockLocation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tiling", meta = (AllowPrivateAccess = "true"))
	int32 MinTilesPerShake = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tiling", meta = (AllowPrivateAccess = "true"))
	int32 MaxTilesPerShake = 4;
	UPROPERTY(EditDefaultsOnly, Category = "Tiling")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, Category="Effects")
	TSubclassOf<UCameraShake> WarningShake;
	UPROPERTY(EditAnywhere, Category="Effects")
	float ShakeTime = 1.5f;
	UPROPERTY(EditAnywhere, Category="Effects")
	float Downtime = 3.5f;
	UPROPERTY(EditAnywhere, Category="Effects")
	USoundBase* ExplosionSound;
	UPROPERTY(EditAnywhere, Category="Effects")
	USoundBase* EarthquakeSound;
	UAudioComponent* EarthquakeAudio;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Final Block Spawning", meta = (AllowPrivateAccess = "true"))
	float FinalBlockSpawnDelay = 1.0f;
	FTimerHandle ExitSpawnTimerHandle;

	TArray<ACaveTile*> Tiles;
	ACaveTile* FinalBlockRef;
	ABaseCharacter* PlayerRef;
	int32 NumFilledTiles;
	FTimerHandle TileTimerHandle;
	bool IsGameOver;
	bool PlayerWin;
	bool MiddleToEndFilled;

	void SpawnFinalBlock();
	void StartShake();
	void TileSpawnTick();
	bool BruteForceSpawn();
	bool SpawnTile(int32 X, int32 Y, bool BruteForceFlag);
	int32 GetIndex(int32 X, int32 Y) const;

};