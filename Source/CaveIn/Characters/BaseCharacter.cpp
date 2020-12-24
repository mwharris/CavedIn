#include "BaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "CaveIn/Actors/CaveTile.h"
#include "CaveIn/Components/HealthComponent.h"
#include "CaveIn/GameModes/CaveInGameMode.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	CanAttack = true;
	AttackHeld = false;

	PickaxeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pickaxe Mesh"));
	PickaxeMesh->SetupAttachment(RootComponent);
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABaseCharacter::OnOverlapBegin);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Get a reference to our Game Mode
	GameModeRef = Cast<ACaveInGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	// Get a reference to our Player Controller
    PlayerControllerRef = GetController<APlayerController>();
	if (PlayerControllerRef) 
	{
		PlayerControllerRef->bShowMouseCursor = true;
	}
	// Setup our pickaxe attack timeline
	SetupTimeline();
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveUp", this, &ABaseCharacter::MoveUp);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABaseCharacter::MoveRight);
	PlayerInputComponent->BindAction("Attack", EInputEvent::IE_Pressed, this, &ABaseCharacter::AttackPressed);
	PlayerInputComponent->BindAction("Attack", EInputEvent::IE_Released, this, &ABaseCharacter::AttackReleased);
	PlayerInputComponent->BindAction("Restart", EInputEvent::IE_Pressed, this, &ABaseCharacter::RestartLevel);
	PlayerInputComponent->BindAction("Quit", EInputEvent::IE_Pressed, this, &ABaseCharacter::QuitGame);
}

void ABaseCharacter::SetupTimeline() 
{
	// Timeline will call this function when it's running
	FOnTimelineFloat TimelineCallback;
	TimelineCallback.BindUFunction(this, FName(TEXT("ControlAttack")));
	// Timeline will call this function when it's completed running
	FOnTimelineEventStatic TimelineFinishedCallback;
	TimelineFinishedCallback.BindUFunction(this, FName(TEXT("SetAttackState")));
	// Setup the timeline to call the above functions 
	AttackTimeline.AddInterpFloat(AttackCurve, TimelineCallback);
	AttackTimeline.SetTimelineFinishedFunc(TimelineFinishedCallback);
}

void ABaseCharacter::Tick(float DeltaTime) 
{
	// Tick our attack timeline if it's running
	if (!CanAttack)
	{
		AttackTimeline.TickTimeline(DeltaTime);
	}
	// Perform a raycast under our mouse to find our mouse location in the world
	if (PlayerControllerRef) 
	{
		FHitResult TraceHitResult;
		PlayerControllerRef->GetHitResultUnderCursor(ECC_Visibility, false, TraceHitResult);
		FVector HitLocation = TraceHitResult.ImpactPoint;
		// Rotate the player towards this location
		Rotate(HitLocation);
	}
	// Tick our Attack if we're currently attacking
	Attack();
}

void ABaseCharacter::MoveUp(float AxisValue) 
{
	GetCharacterMovement()->AddInputVector((-FVector::RightVector) * AxisValue * MovementSpeed * GetWorld()->DeltaTimeSeconds);
}

void ABaseCharacter::MoveRight(float AxisValue) 
{
	GetCharacterMovement()->AddInputVector(FVector::ForwardVector * AxisValue * MovementSpeed * GetWorld()->DeltaTimeSeconds);
}

void ABaseCharacter::Rotate(FVector LookAtTarget)
{
	// "Clean" our lookat target by making sure the point is on the same Z as the actor
	FVector LookAtTargetClean = FVector(LookAtTarget.X, LookAtTarget.Y, GetActorLocation().Z);
	FVector StartLocation = GetActorLocation();
	// Subtract our LookAtTarget from our Location to get the vector pointing from Player->LookAt
	FRotator Rotator = FVector(LookAtTargetClean - StartLocation).Rotation();
	// Update our Rotation to match this vector
	PlayerControllerRef->SetControlRotation(Rotator);
}

void ABaseCharacter::Attack() 
{
	if (!CanAttack || !AttackHeld) { return; }
	CanAttack = false;
	// Start the attack animation timeline
	AttackTimeline.PlayFromStart();
	// Make sure we get a reference to our owner
	AActor* MyOwner = GetOwner();
	if (!MyOwner)
	{
		UE_LOG(LogTemp, Error, TEXT("Attack: No Owner Found!"));
		return;
	}
	// Do a simple raycast attack forward for now
	FVector Start = GetActorLocation();
	FVector End = GetActorLocation() + (GetActorForwardVector() * AttackDistance);
	FHitResult OutHit;
	bool Success = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility);
	if (Success && OutHit.GetActor() != NULL && OutHit.GetActor() != this)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SparksSystem, OutHit.ImpactPoint, OutHit.ImpactNormal.Rotation());
		UGameplayStatics::SpawnSoundAtLocation(this, AttackSound, GetActorLocation());
		UGameplayStatics::ApplyDamage(OutHit.GetActor(), AttackDamage, MyOwner->GetInstigatorController(), this, DamageType);
	}
}

void ABaseCharacter::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) 
{
	// Destroy any Cave Tiles that fall on our heads 
	ACaveTile* Tile = Cast<ACaveTile>(OtherActor);
	if (OtherActor && OtherActor != this && Tile)
	{
		if (Tile->IsFalling())
		{
			GameModeRef->ActorDied(OtherActor);
		}
	}
}

void ABaseCharacter::ApplyDamageToFinalBlock(ACaveTile* FinalBlock) 
{
	if (FinalBlock) 
	{
		AActor* MyOwner = GetOwner();
		if (!MyOwner)
		{
			UE_LOG(LogTemp, Error, TEXT("ApplyDamageToFinalBlock: No Owner Found!"));
			return;
		}
		if (FinalBlock->ExplosionPoint) 
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionParticle, FinalBlock->ExplosionPoint->GetComponentLocation());
		}
		UGameplayStatics::ApplyDamage(FinalBlock, BombDamage, MyOwner->GetInstigatorController(), this, DamageType);
	}
}

void ABaseCharacter::ControlAttack() 
{
	FVector Location = GetActorLocation();
	// Get the current time value
	TimelineValue = AttackTimeline.GetPlaybackPosition();
	// Get the current float value from the timeline using the time value above
	CurveFloatValue = AttackCurve->GetFloatValue(TimelineValue);
	// Set our new location given the timeline values
	FRotator CurrRotation = PickaxeMesh->GetRelativeRotation();
	FRotator NewRotation = FRotator(CurveFloatValue, CurrRotation.Yaw, CurrRotation.Roll);
	PickaxeMesh->SetRelativeRotation(NewRotation);
}

void ABaseCharacter::SetAttackState() 
{
	CanAttack = true;
}

void ABaseCharacter::AttackPressed()
{
	AttackHeld = true;
}

void ABaseCharacter::AttackReleased() 
{
	AttackHeld = false;
}

void ABaseCharacter::RestartLevel() 
{
	UGameplayStatics::OpenLevel(GetWorld(), "Level1", true);
}

void ABaseCharacter::QuitGame() 
{
	UGameplayStatics::OpenLevel(GetWorld(), "SplashScreen", true);
}

float ABaseCharacter::GetHealth() const
{
	return HealthComponent->GetHealth();
}

bool ABaseCharacter::IsDead() const
{
	return HealthComponent->IsDead();
}