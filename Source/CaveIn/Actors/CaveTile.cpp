#include "CaveTile.h"
#include "CaveIn/Components/HealthComponent.h"
#include "CaveIn/Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Curves/CurveFloat.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ACaveTile::ACaveTile()
{
	PrimaryActorTick.bCanEverTick = true;
	Indestructible = false;
	Falling = true;
	BombBlock = false;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	SetRootComponent(BoxCollider);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	StaticMesh->SetupAttachment(RootComponent);
	
	TextRenderer = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Health Text"));
	TextRenderer->SetupAttachment(RootComponent);

	ExplosionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Explosion Point"));
	ExplosionPoint->SetupAttachment(RootComponent);

	IndestructibleMaterial = CreateDefaultSubobject<UMaterial>("Indestructible Material");
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
}

void ACaveTile::BeginPlay()
{
	Super::BeginPlay();
	// Final Block gets it's own material
	if (FinalBlock)
	{
		HealthComponent->SetDefaultHealth(HealthComponent->GetDefaultHealth() * FinalBlockHealthMultiplier);
		HealthComponent->SetHealth(HealthComponent->GetHealth() * FinalBlockHealthMultiplier);
		StaticMesh->SetMaterial(0, Cast<UMaterialInterface>(FinalBlockMaterial));
	}
	// Indestructible block cannot be destroyed, get a new material
	else if (Indestructible) 
	{
		TextRenderer->DestroyComponent();
		HealthComponent->SetIndestructible(true);
		StaticMesh->SetMaterial(0, Cast<UMaterialInterface>(IndestructibleMaterial));
	}
	else if (BombBlock)
	{
		TextRenderer->DestroyComponent();
		StaticMesh->SetMaterial(0, Cast<UMaterialInterface>(BombBlockMaterial));
	}
	else 
	{
		TextRenderer->DestroyComponent();
	}
	// Setup our falling timeline
	SetupTimeline();
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

void ACaveTile::Tick(float DeltaTime) 
{
	// Tick our fall timeline if we're falling
	if (Falling)
	{
		MyTimeline.TickTimeline(DeltaTime);
	}
	if (FinalBlock) 
	{
		TextRenderer->SetText(FText::AsNumber(GetHealth() / 50.f));
	}
}

// Called while the FallTimeline is running
void ACaveTile::ControlFall() 
{
	UE_LOG(LogTemp, Warning, TEXT("ControlFall!"));
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

// Called when the Timeline finished to reset our state
void ACaveTile::SetFallState() 
{
	Falling = false;
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), SlamSound, GetActorLocation(), 0.5f);
}

void ACaveTile::HandleDestruction() 
{
	Destroy();
}

float ACaveTile::GetHealth() const
{
	return HealthComponent->GetHealth();
}

void ACaveTile::SetHealth(float Health) 
{
	HealthComponent->SetHealth(Health);
}

bool ACaveTile::IsDead() const
{
	return HealthComponent->IsDead();
}

bool ACaveTile::IsFalling() const
{
	return Falling;
}

void ACaveTile::SetIndestructible(bool Value) 
{
	Indestructible = Value;
}

bool ACaveTile::GetIsBombBlock() const
{
	return BombBlock;
}

void ACaveTile::SetIsBombBlock(bool Value) 
{
	BombBlock = Value;
}

bool ACaveTile::GetIndestructible() const
{
	return Indestructible;	
}

bool ACaveTile::GetIsFinalBlock() const
{
	return FinalBlock;
}

void ACaveTile::SetIsFinalBlock(bool Value) 
{
	FinalBlock = Value;
}