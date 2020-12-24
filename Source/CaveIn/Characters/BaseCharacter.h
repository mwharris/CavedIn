#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "BaseCharacter.generated.h"

class UCameraComponent;
class UHealthComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UParticleSystem;
class UNiagaraSystem;
class ACaveInGameMode;
class ACaveTile;

UCLASS()
class CAVEIN_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UHealthComponent* HealthComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* PickaxeMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float MovementSpeed = 10;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float Deadzone = 0.25;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attack")
	float AttackDistance = 20;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attack")
	float AttackDamage = 50;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attack")
	float BombDamage = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attack")
	float AttackRateSeconds = 1;
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	TSubclassOf<UDamageType> DamageType;
	UPROPERTY(EditAnywhere, Category="Attack")
	UCurveFloat* AttackCurve;
	UPROPERTY(EditAnywhere, Category="Effects")
	UParticleSystem* ExplosionParticle;
	UPROPERTY(EditAnywhere, Category="Effects")
	UNiagaraSystem* SparksSystem;


	UFUNCTION()
	void ControlAttack();
	UFUNCTION()
	void SetAttackState();	
	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void ApplyDamageToFinalBlock(ACaveTile* FinalBlock);
	float GetHealth() const;
	bool IsDead() const;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category="Effects")
	USoundBase* AttackSound;

	APlayerController* PlayerControllerRef;
	ACaveInGameMode* GameModeRef;
	FTimerHandle AttackTimer;
	bool CanAttack;
	bool AttackHeld;
	float CurveFloatValue;
	float TimelineValue;
	FTimeline AttackTimeline;

	void MoveUp(float AxisValue);
	void MoveRight(float AxisValue);
	void Attack();
	void AttackPressed();
	void AttackReleased();
	void Rotate(FVector LookAtTarget);
	void RestartLevel();
	void QuitGame();
	void SetupTimeline();

};
