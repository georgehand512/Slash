// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "Characters/CharacterTypes.h"
#include "Interfaces/PickupInterface.h"
#include "SlashCharacter.generated.h"

// forward declares
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UGroomComponent;
class AItem;
class AWeapon;
class ASoul;
class ATreasure;
class UAnimMontage;
class UHUDOverlay;


UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	ASlashCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Jump() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void AddSouls(ASoul* Soul) override;
	virtual void AddGold(ATreasure* Treasure) override;

protected:
	virtual void BeginPlay() override;

	/** Callback for user input	*/
	void Move(const FInputActionValue& Value);
	void Walk();	
	void Run();	
	void Look(const FInputActionValue& Value);
	void EKeyPressed();
	virtual void Attack() override;
	void RandomAttack();
	void SpecificAttack();
	void Dodge();

	/** Combat	*/
	void EquipWeapon(AWeapon* Weapon);
	virtual bool CanAttack() override;
	virtual void DodgeEnd() override;
	virtual void AttackEnd() override;
	bool CanDisarm();
	bool CanArm();
	void Disarm();
	void Arm();
	void PlayEquipMontage(const FName SectionName);
	virtual void Die_Implementation() override;

	UPROPERTY(EditAnywhere, Category = "Input" )
	UInputMappingContext* SlashMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input" )
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, Category = "Input" ) 	
	UInputAction* WalkRunAction;					

	UPROPERTY(EditAnywhere, Category = "Input" )
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input" )
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input" )
	UInputAction* EKeyAction;

	UPROPERTY(EditAnywhere, Category = "Input" )
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category = "Input" )
	UInputAction* SpecificAttackAction;

	UPROPERTY(EditAnywhere, Category = "Input" )
	UInputAction* DodgeAction;

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack();

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

private:
	void SetCombatTargetToClosestEnemyInRange();
	bool IsUnoccuppied();
	bool IsOccupied();
	bool HasEnoughStamina();
	void InitializeHUDOverlay();
	void SetHUDHealth();
	void SetHUDStamina();
	void SetHUDGold();
	void SetHUDSouls();

	/** Character Components	*/
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(EditAnywhere, Category = "Input")	
	float WalkSpeed = 200.f;						

	UPROPERTY(EditAnywhere, Category = "Input")	
	float RunSpeed = 600.f;							

	UPROPERTY(VisibleAnywhere, Category = "Hair")
	UGroomComponent* Hair;

	UPROPERTY(VisibleAnywhere, Category = "Hair")
	UGroomComponent* Eyebrows;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* EquipMontage;

	
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY()
	UHUDOverlay* HUDOverlay;

public:
	/** Setters & Getters	*/
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
	FORCEINLINE float GetWalkSpeed() const { return WalkSpeed; }	
	FORCEINLINE float GetRunSpeed() const { return RunSpeed; }		
};












