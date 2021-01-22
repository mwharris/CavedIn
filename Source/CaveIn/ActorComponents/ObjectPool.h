#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectPool.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CAVEIN_API UObjectPool : public UActorComponent
{
	GENERATED_BODY()

public:	
	UObjectPool();
	void DoSomething();

protected:
	virtual void BeginPlay() override;
		
};
