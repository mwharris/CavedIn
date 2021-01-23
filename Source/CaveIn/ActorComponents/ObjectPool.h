#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectPool.generated.h"

class APooledActor;

UCLASS( ClassGroup=(Custom), Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent) )
class CAVEIN_API UObjectPool : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pooler")
	TSubclassOf<APooledActor> PooledObjectSubclass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pooler")
	int32 PoolSize = 100;

	UObjectPool();
	void InitPool();
	APooledActor* GetPooledObject();

protected:
	virtual void BeginPlay() override;

private:
	TArray<APooledActor*> Pool;
		
};