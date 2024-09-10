// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/HitInterface.h"
#include "Animation/AnimMontage.h"
#include "NiagaraComponent.h"


AWeapon::AWeapon()
{
	WeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollissionBox"));
	WeaponCollisionBox->SetupAttachment(GetRootComponent());
    WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponCollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    WeaponCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

    BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("BoxTrace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());

    BoxTraceStop = CreateDefaultSubobject<USceneComponent>(TEXT("BoxTrace Stop"));
	BoxTraceStop->SetupAttachment(GetRootComponent());
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    WeaponCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::WeaponBoxOverlap);
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
    ItemState = EItemState::EIS_Equipped;	
    SetOwner(NewOwner);
	SetInstigator(NewInstigator);
    
    AttachMeshToSocket(InParent, InSocketName);
    DisableSphereCollision();
    
    PlayEquipSound();
    DeactivateEmbers();
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName)
{
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
    ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
}

void AWeapon::WeaponBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (IsActorSameTypeAs(OtherActor)) return; //early return to stop enemies hit each other
    
    FHitResult OutHit;
    BoxTrace (OutHit);

    if (OutHit.GetActor())
    {
        if (IsActorSameTypeAs(OutHit.GetActor())) return; //early return to stop enemies hit each other
        UGameplayStatics::ApplyDamage(OutHit.GetActor(), Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
        ExecuteGetHit(OutHit);
        CreateFields(OutHit.ImpactPoint);
    }
}

void AWeapon::PlayEquipSound()
{
        if (EquipSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
    }
}

void AWeapon::DisableSphereCollision()
{
    if (Sphere)
    {
        Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AWeapon::DeactivateEmbers()
{
    if (ItemEffect)
    {
        ItemEffect->Deactivate();
    }
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
    const FVector Start = BoxTraceStart->GetComponentLocation();
    const FVector End = BoxTraceStop->GetComponentLocation();
    
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);
    ActorsToIgnore.Add(GetOwner());

    // Add actor just hit so only has one hit per attack
    for (AActor* Actor : IgnoreActors)
    {
        ActorsToIgnore.AddUnique(Actor);
    }

    UKismetSystemLibrary::BoxTraceSingle(
        this,
        Start,
        End,
        BoxTraceExtent,
        BoxTraceStart->GetComponentRotation(),
        ETraceTypeQuery::TraceTypeQuery1,
        false,
        ActorsToIgnore,
        bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
        BoxHit,
        true);

    IgnoreActors.AddUnique(BoxHit.GetActor()); 
}

void AWeapon::ExecuteGetHit(FHitResult& BoxHit)
{
    IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor());
        if (HitInterface)
        {
            HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, GetOwner());
        }
}

bool AWeapon::IsActorSameTypeAs(AActor* OtherActor)
{
    return GetOwner()->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"));
}