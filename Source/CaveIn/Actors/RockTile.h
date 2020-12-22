#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RockTile.generated.h"

class UBoxComponent;
class UHealthComponent;
class UMaterial;
class UStaticMeshComponent;
class USceneComponent;
class UTextRenderComponent;

UCLASS()
class CAVEIN_API ARockTile : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* BoxCollider;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* StaticMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* ExplosionPoint;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTextRenderComponent* TextRenderer;

	ARockTile();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

};