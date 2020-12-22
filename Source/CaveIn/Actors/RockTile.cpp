#include "RockTile.h"
#include "CaveIn/Components/HealthComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"

ARockTile::ARockTile()
{
	PrimaryActorTick.bCanEverTick = true;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	SetRootComponent(BoxCollider);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	StaticMesh->SetupAttachment(RootComponent);
	
	TextRenderer = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Health Text"));
	TextRenderer->SetupAttachment(RootComponent);

	ExplosionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Explosion Point"));
	ExplosionPoint->SetupAttachment(RootComponent);
}

void ARockTile::BeginPlay()
{
	Super::BeginPlay();
}

void ARockTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}