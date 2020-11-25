#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "AttackBox.generated.h"

class AActor;

UCLASS()
class CAVEIN_API AAttackBox : public ATriggerBox
{
	GENERATED_BODY()
	
public:	
	AAttackBox();

	UPROPERTY()
	bool Attacking;

	UFUNCTION()
	void Attack(AActor* OverlappedActor, AActor* OtherActor);

protected:
	virtual void BeginPlay() override;

private:
	float Damage = 50;

};