#include "CaveTile.h"
#include "CaveIn/Components/HealthComponent.h"
#include "CaveIn/Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Curves/CurveFloat.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

ACaveTile::ACaveTile()
{
	PrimaryActorTick.bCanEverTick = false;
	Falling = false;
	Indestructible = false;
	BombBlock = false;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	SetRootComponent(BoxCollider);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	StaticMesh->SetupAttachment(RootComponent);
	
	TextRenderer = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Health Text"));
	TextRenderer->SetupAttachment(RootComponent);

	ExplosionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Explosion Point"));
	ExplosionPoint->SetupAttachment(RootComponent);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
}

void ACaveTile::BeginPlay()
{
	Super::BeginPlay();
}

void ACaveTile::InitTile(FVector Location, FRotator Rotation, bool bIndestructible, bool bIsBomb, bool bFinalBlock) 
{
	SetActorLocation(Location);
	SetActorRotation(Rotation);
	SetActorEnableCollision(true);
	SetActive(true);
	SetIndestructible(bIndestructible);
	SetIsBombBlock(bIsBomb);
	SetIsFinalBlock(bFinalBlock);
	StartFalling();
}

void ACaveTile::SetupTimeline() 
{
	// Timeline will call this function when it's running
	FOnTimelineFloat TimelineCallback;
	TimelineCallback.BindUFunction(this, FName(TEXT("ControlFall")));
	// Timeline will call this function when it's completed running
	FOnTimelineEventStatic TimelineFinishedCallback;
	TimelineFinishedCallback.BindUFunction(this, FName(TEXT("SetFallState")));
	// Setup the timeline to call the above functions 
	MyTimeline.AddInterpFloat(FallCurve, TimelineCallback);
	MyTimeline.SetTimelineFinishedFunc(TimelineFinishedCallback);
	// Start the Timeline
	MyTimeline.PlayFromStart();
}

// Tick that runs on FallTimerHandle only while the CaveTile is falling
void ACaveTile::TickTimeline() 
{
	if (Falling)
	{
		MyTimeline.TickTimeline(FallTimerFrequency);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(FallTimerHandle);
	}
}

// Called while the FallTimeline is running
void ACaveTile::ControlFall() 
{
	FVector Location = GetActorLocation();
	// Get the current time value
	TimelineValue = MyTimeline.GetPlaybackPosition();
	// Get the current float value from the timeline using the time value above
	CurveFloatValue = FallCurve->GetFloatValue(TimelineValue);
	// Set our new location given the timeline values
	if (FinalBlock && CurveFloatValue < 200) 
	{
		CurveFloatValue = 200;
	}
	FVector NewLocation = FVector(Location.X, Location.Y, CurveFloatValue);
	SetActorLocation(NewLocation);
}

// Called when the tile is spawned to start the process of falling
void ACaveTile::StartFalling() 
{
	SetupTimeline();
	Falling = true;
	GetWorldTimerManager().SetTimer(FallTimerHandle, this, &ACaveTile::TickTimeline, FallTimerFrequency, true);
}

// Called when the Timeline finished to reset our state
void ACaveTile::SetFallState() 
{
	Falling = false;
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), SlamSound, GetActorLocation(), 0.5f);
}

void ACaveTile::Reset() 
{
	FinalBlock = false;
	Indestructible = false;
	BombBlock = false;
	Falling = false;
	StaticMesh->SetMaterial(0, Cast<UMaterialInterface>(BaseMaterial));
	SetHealth(HealthComponent->GetDefaultHealth());
	Super::SetActive(false);
}

void ACaveTile::SetIndestructible(bool Value) 
{
	Indestructible = Value;
	if (Indestructible) 
	{
		HealthComponent->SetIndestructible(true);
		StaticMesh->SetMaterial(0, Cast<UMaterialInterface>(IndestructibleMaterial));
		if (TextRenderer != nullptr) 
		{
			TextRenderer->DestroyComponent();
		}
	}
}

void ACaveTile::SetIsBombBlock(bool Value) 
{
	BombBlock = Value;
	if (BombBlock)
	{
		StaticMesh->SetMaterial(0, Cast<UMaterialInterface>(BombBlockMaterial));
		if (TextRenderer != nullptr) 
		{
			TextRenderer->DestroyComponent();
		}
	}
}

void ACaveTile::SetIsFinalBlock(bool Value) 
{
	FinalBlock = Value;
	if (FinalBlock)
	{
		HealthComponent->SetDefaultHealth(HealthComponent->GetDefaultHealth() * FinalBlockHealthMultiplier);
		HealthComponent->SetHealth(HealthComponent->GetHealth() * FinalBlockHealthMultiplier);
		StaticMesh->SetMaterial(0, Cast<UMaterialInterface>(FinalBlockMaterial));
		UpdateDisplayedHealth(GetHealth());
		Super::SetActive(true);
	}
	else if (TextRenderer != nullptr)
	{
		TextRenderer->DestroyComponent();
	}
}

float ACaveTile::GetHealth() const
{
	return HealthComponent->GetHealth();
}

void ACaveTile::SetHealth(float Health) 
{
	HealthComponent->SetHealth(Health);
}

void ACaveTile::UpdateDisplayedHealth(float Health) 
{
	TextRenderer->SetText(FText::AsNumber(Health / 50.f));
}

bool ACaveTile::IsDead() const
{
	return HealthComponent->IsDead();
}

bool ACaveTile::IsFalling() const
{
	return Falling;
}

bool ACaveTile::GetIsBombBlock() const
{
	return BombBlock;
}

bool ACaveTile::GetIndestructible() const
{
	return Indestructible;	
}

bool ACaveTile::GetIsFinalBlock() const
{
	return FinalBlock;
}