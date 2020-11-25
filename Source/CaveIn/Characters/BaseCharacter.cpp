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
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveUp", this, &ABaseCharacter::MoveUp);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABaseCharacter::MoveRight);
	PlayerInputComponent->BindAction("Attack", EInputEvent::IE_Pressed, this, &ABaseCharacter::Attack);
	PlayerInputComponent->BindAction("Restart", EInputEvent::IE_Pressed, this, &ABaseCharacter::RestartLevel);
}

void ABaseCharacter::Tick(float DeltaTime) 
{
	if (PlayerControllerRef) 
	{
		// Perform a raycast under our mouse to find our mouse location in the world
		FHitResult TraceHitResult;
		PlayerControllerRef->GetHitResultUnderCursor(ECC_Visibility, false, TraceHitResult);
		FVector HitLocation = TraceHitResult.ImpactPoint;
		// Rotate the player towards this location
		Rotate(HitLocation);
	}
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
	if (CanAttack) 
	{
		CanAttack = false;
		GetWorldTimerManager().SetTimer(AttackTimer, this, &ABaseCharacter::ResetAttack, AttackRateSeconds, false);
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

void ABaseCharacter::ResetAttack() 
{
	CanAttack = true;
}

void ABaseCharacter::RestartLevel() 
{
	if (!GameModeRef->GetIsGameOver()) { return; }
	UGameplayStatics::OpenLevel(GetWorld(), "Level1", true);
}

float ABaseCharacter::GetHealth() const
{
	return HealthComponent->GetHealth();
}

bool ABaseCharacter::IsDead() const
{
	return HealthComponent->IsDead();
}