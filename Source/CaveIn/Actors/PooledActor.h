#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PooledActor.generated.h"

UCLASS()
class CAVEIN_API APooledActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APooledActor();
	void SetActive(bool NewValue);
	bool IsActive();
	
	virtual void Reset() 
	{
		SetActive(false);
	};

protected:
	bool Active = false;

	virtual void BeginPlay() override;

};
