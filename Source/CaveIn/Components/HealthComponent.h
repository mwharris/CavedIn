#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class ACaveInGameMode;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CAVEIN_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();
	float GetHealth() const;
	void SetHealth(float NewHealth); 
	float GetDefaultHealth() const;
	void SetDefaultHealth(float NewDefaultHealth);
	bool IsDead() const;
	void SetIndestructible(bool Immortal);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

private:
	UPROPERTY(EditAnywhere)
	float DefaultHealth = 100.f;
	UPROPERTY(VisibleAnywhere)	
	float Health = 0.f;

	ACaveInGameMode* GameModeRef;
	bool Indestructible;
		
};
