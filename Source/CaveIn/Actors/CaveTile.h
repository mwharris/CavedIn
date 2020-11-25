#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "CaveTile.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UHealthComponent;
class UMaterial;
class UTextRenderComponent;
class USceneComponent;

UCLASS()
class CAVEIN_API ACaveTile : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* BoxCollider;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* StaticMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UHealthComponent* HealthComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* ExplosionPoint;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextRenderComponent* TextRenderer;
	UPROPERTY(EditAnywhere, Category="Tile Materials")
	UMaterial* IndestructibleMaterial;
	UPROPERTY(EditAnywhere, Category="Tile Materials")
	UMaterial* FinalBlockMaterial;
	UPROPERTY(EditAnywhere, Category="Tile Materials")
	UMaterial* BombBlockMaterial;
	UPROPERTY(EditAnywhere, Category="Final Block")
	float FinalBlockHealthMultiplier = 50;
	UPROPERTY(EditAnywhere, Category="Falling")
	UCurveFloat* FallCurve;
	UPROPERTY(EditAnywhere, Category="Falling")
	USoundBase* SlamSound;

	UFUNCTION()
	void ControlFall();
	UFUNCTION()
	void SetFallState();

	ACaveTile();
	virtual void Tick(float DeltaTime) override;
	void HandleDestruction();
	
	// GETTERS AND SETTERS
	bool GetIsFinalBlock() const;
	void SetIsFinalBlock(bool Value);
	bool GetIndestructible() const;
	void SetIndestructible(bool Value);
	bool GetIsBombBlock() const;
	void SetIsBombBlock(bool Value);
	float GetHealth() const;
	void SetHealth(float Health);
	bool IsDead() const;
	bool IsFalling() const;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	float StopZ = 100;

	bool FinalBlock;
	bool Indestructible;
	bool BombBlock;
	bool Falling;
	float CurveFloatValue;
	float TimelineValue;
	FTimeline MyTimeline;
	int32 TileIndex;

	void SetupTimeline();

};
