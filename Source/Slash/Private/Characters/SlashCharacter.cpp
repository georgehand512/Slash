// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/SlashCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/staticMeshComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GroomComponent.h"
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "HUD/SlashHUD.h"
#include "HUD/HUDOverlay.h"
#include "Components/AttributeComponent.h"
#include "Items/Treasure.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Math/Vector.h"


// Sets default values
ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;
	// Makes the Spring arm rotate with the Look() function
	SpringArm->bUsePawnControlRotation = true;
	// Rotate the camera above character
	SpringArm->AddRelativeRotation(FRotator(-20.f, 0.f, 0.f));	
 
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("Head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("Head");
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (Attributes && HUDOverlay)
	{
		Attributes->RegenStamina(DeltaTime);
		HUDOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}


		SetCombatTargetToClosestEnemyInRange();

	
}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(WalkRunAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Run); 
		EnhancedInputComponent->BindAction(WalkRunAction, ETriggerEvent::Completed, this, &ASlashCharacter::Walk); 
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &ASlashCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::RandomAttack);
		EnhancedInputComponent->BindAction(SpecificAttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::SpecificAttack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Dodge);
	}
}

void ASlashCharacter::Jump()
{
	if (IsUnoccuppied())
	{
		Super::Jump();
	}
}

float ASlashCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();
	SetHUDHealth();
	return DamageAmount;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);

	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Attributes && Attributes->GetHealthPercent() > 0.f)
	{
		ActionState = EActionState::EAS_HitReaction;
	}
	
}

void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;	
}

void ASlashCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes && HUDOverlay)
	{
		Attributes->AddSouls(Soul->GetSouls());
		HUDOverlay->SetSouls(Attributes->GetSouls());
	}
}

void ASlashCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes && HUDOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		HUDOverlay->SetGold(Attributes->GetGold());
	}
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(FName("EngageableCharacter"));

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashMappingContext, 0);
		}
	}

	InitializeHUDOverlay();
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;
	
	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(ForwardDirection, MovementVector.Y);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASlashCharacter::Walk()
{
	if (ActionState != EActionState::EAS_Unoccupied) return;
	
	if (Controller)
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void ASlashCharacter::Run()
{
	if (ActionState != EActionState::EAS_Unoccupied) return;
	
	if (Controller)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller)
	{
		AddControllerPitchInput(LookAxisVector.Y);
		AddControllerYawInput(LookAxisVector.X);		
	}	
}

void ASlashCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);

	if (OverlappingWeapon && CharacterState == ECharacterState::ECS_Unequipped)
	{
		if (EquippedWeapon) EquippedWeapon->Destroy();

		EquipWeapon(OverlappingWeapon);
	}
	else 
	{
		if (CanDisarm())
		{
			Disarm();
		}
		else if (CanArm())
		{
			Arm();			
		}
	}
}

void ASlashCharacter::Attack()
{
	Super::Attack();
}

void ASlashCharacter::RandomAttack()
{
	Attack();

	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::SpecificAttack()
{
	Attack();

	if (CanAttack())
	{
		PlaySpecificAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::Dodge()
{
	if (IsOccupied() || !HasEnoughStamina()) return;
	ActionState = EActionState::EAS_Dodge;
	PlayDodgeMontage(); 	
	Attributes->UseStamina(Attributes->GetDodgeStaminaCost());
	if (HUDOverlay)	HUDOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
}

void ASlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	OverlappingItem = nullptr;
	EquippedWeapon = Weapon;
}

bool ASlashCharacter::CanAttack()
{
	return (ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped);
}

void ASlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();

	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::AttackEnd()
{
	Super::AttackEnd();
	
	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanDisarm()
{
	return 	ActionState == EActionState::EAS_Unoccupied && 
			CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::CanArm()
{
	return 	ActionState == EActionState::EAS_Unoccupied &&
			CharacterState == ECharacterState::ECS_Unequipped && EquippedWeapon;
}

void ASlashCharacter::Disarm()
{
	PlayEquipMontage(FName("Unequip"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::Arm()
{
	PlayEquipMontage(FName("Equip"));
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::PlayEquipMontage(const FName SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void ASlashCharacter::Die_Implementation()
{
	Super::Die_Implementation();

	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();
}

void ASlashCharacter::AttachWeaponToBack()
{
	if(EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void ASlashCharacter::AttachWeaponToHand()
{
	if(EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::SetCombatTargetToClosestEnemyInRange()
{
	const FVector CharacterPos = GetActorLocation();

	// Set what actors to seek out from it's collision channel
	TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes;
	traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);
 //   ActorsToIgnore.Add(GetOwner());

	TArray<AActor*> OutActors;

	// perform sphere Trace with combat radius
	UKismetSystemLibrary::SphereOverlapActors
	(
    	this,
    	CharacterPos,
    	CombatRadius,
    	traceObjectTypes,
   		NULL,
    	ActorsToIgnore,
    	OutActors
	);

	double ClosestDistance = CombatRadius;
	AActor* ClosestTarget = nullptr;
	
	if (OutActors.Num() != 0)
	{
		for (int32 Index = 0; Index != OutActors.Num(); ++Index)
    	{
      		double DistanceToTarget = (CharacterPos - OutActors[Index]->GetActorLocation()).Size();
				
			if (DistanceToTarget < ClosestDistance && OutActors[Index]->ActorHasTag(FName("Enemy")))
			{
				ClosestDistance = DistanceToTarget;
				ClosestTarget = OutActors[Index];		
			}
    	}
	}

	CombatTarget = ClosestTarget;

	// screen meesage
	if (CombatTarget != nullptr)
	{
		const FString EnemyName = CombatTarget->GetName();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage (1, 2.f, FColor::Red, EnemyName );
		}	
	}
	else 
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage (1, 2.f, FColor::Red, TEXT("Null pointer no enemy in range") );
		}	
	}

	
}

bool ASlashCharacter::IsUnoccuppied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::HasEnoughStamina()
{
	return Attributes && (Attributes->GetStamina() >= Attributes->GetDodgeStaminaCost());
}

void ASlashCharacter::InitializeHUDOverlay()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD()))
		{
			HUDOverlay = SlashHUD->GetHUDOverlay();
			if (HUDOverlay && Attributes)
			{
				HUDOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
				HUDOverlay->SetHealthBarPercent(1.f);
				HUDOverlay->SetGold(0);
				HUDOverlay->SetSouls(0);
			}
		}
	}
}

void ASlashCharacter::SetHUDHealth()
{
	if (HUDOverlay && Attributes)
	{
		HUDOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

void ASlashCharacter::SetHUDStamina()
{

}

void ASlashCharacter::SetHUDGold()
{

}

void ASlashCharacter::SetHUDSouls()
{

}