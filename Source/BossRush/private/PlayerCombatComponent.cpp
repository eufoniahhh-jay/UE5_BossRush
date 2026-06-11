// PlayerCombatComponent.cpp

#include "PlayerCombatComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerStatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UPlayerCombatComponent::UPlayerCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ACharacter>(GetOwner());

    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Owner is not Character"));
        return;
    }

    // stat component 가져오기
    OwnerStatComponent = OwnerCharacter->FindComponentByClass<UPlayerStatComponent>();

    if (!OwnerStatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] OwnerStatComponent is null"));
    }
}

void UPlayerCombatComponent::LightAttack()
{
    /*if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] OwnerCharacter is null"));
        return;
    }

    if (bIsAttacking)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Already attacking"));
        return;
    }

    if (!LightAttackMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] LightAttackMontage is null"));
        return;
    }*/

    if (!CanAttack())
    {
        return;
    }

    UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()
        ? OwnerCharacter->GetMesh()->GetAnimInstance()
        : nullptr;

    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] AnimInstance is null"));
        return;
    }

    const float PlayedLength = AnimInstance->Montage_Play(LightAttackMontage, 1.0f);

    if (PlayedLength <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Failed to play LightAttackMontage"));
        return;
    }

    bIsAttacking = true;
    bAttackHitAlready = false;

    UE_LOG(LogTemp, Warning, TEXT("[Combat] LightAttack started: %s"), *LightAttackMontage->GetName());
}

void UPlayerCombatComponent::HandleAttackStart()
{
    UE_LOG(LogTemp, Warning, TEXT("[Combat] AttackStart Notify"));

    PerformAttackTrace();
}

void UPlayerCombatComponent::HandleAttackEnd()
{
    UE_LOG(LogTemp, Warning, TEXT("[Combat] AttackEnd Notify"));

    bIsAttacking = false;
    bAttackHitAlready = false;
}

void UPlayerCombatComponent::PerformAttackTrace()
{
    if (!OwnerCharacter)
    {
        return;
    }

    if (bAttackHitAlready)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Attack already hit. Skip trace."));
        return;
    }

    const FVector Start = OwnerCharacter->GetActorLocation() + FVector(0.0f, 0.0f, 70.0f);
    const FVector End = Start + OwnerCharacter->GetActorForwardVector() * AttackTraceDistance;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);

    const bool bHit = GetWorld()->SweepSingleByChannel(
        Hit,
        Start,
        End,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(AttackTraceRadius),
        Params
    );

    DrawDebugSphere(GetWorld(), Start, AttackTraceRadius, 16, FColor::Green, false, 1.0f);
    DrawDebugSphere(GetWorld(), End, AttackTraceRadius, 16, FColor::Red, false, 1.0f);
    DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 1.0f, 0, 2.0f);

    if (bHit && Hit.GetActor())
    {
        AActor* HitActor = Hit.GetActor();

        UE_LOG(LogTemp, Warning, TEXT("[Combat] Hit Actor: %s"), *Hit.GetActor()->GetName());

        bAttackHitAlready = true;

        // 보스 HP 감소는 다음 단계에서 연결
        /*FName FunctionName = TEXT("ApplyBossDamage");

        if (UFunction* DamageFunction = HitActor->FindFunction(FunctionName))
        {
            struct FDamageParams
            {
                double DamageAmount;
            };

            FDamageParams DamageParams;
            DamageParams.DamageAmount = static_cast<double>(AttackDamage);

            HitActor->ProcessEvent(DamageFunction, &DamageParams);

            UE_LOG(LogTemp, Warning, TEXT("[Combat] Apply damage: %.1f"), AttackDamage);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[Combat] Hit actor has no ApplyBossDamage function"));
        }*/
        UGameplayStatics::ApplyDamage(
            HitActor,
            AttackDamage,
            OwnerCharacter->GetController(),
            OwnerCharacter,
            nullptr
        );

        UE_LOG(LogTemp, Warning, TEXT("[Combat] Apply damage: %.1f"), AttackDamage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Attack trace missed"));
    }
}

void UPlayerCombatComponent::Dodge()
{
    /*if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Dodge failed: OwnerCharacter is null"));
        return;
    }

    if (!OwnerStatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Dodge failed: OwnerStatComponent is null"));
        return;
    }

    if (bIsAttacking)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Dodge failed: currently attacking"));
        return;
    }

    if (bIsDodging)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Dodge failed: already dodging"));
        return;
    }

    if (!OwnerStatComponent->HasEnoughStamina(DodgeStaminaCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Dodge failed: not enough stamina"));
        return;
    }*/

    if (!CanDodge())
    {
        return;
    }

    OwnerStatComponent->ConsumeStamina(DodgeStaminaCost);

    bIsDodging = true;
    bIsInvincible = false;

    UE_LOG(LogTemp, Warning, TEXT("[Combat] Dodge started"));

    // 임시 회피 이동
    const FVector DodgeDirection = OwnerCharacter->GetActorForwardVector();
    OwnerCharacter->LaunchCharacter(DodgeDirection * DodgeStrength, true, false);

    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_InvincibleStart,
        FTimerDelegate::CreateUObject(this, &UPlayerCombatComponent::SetInvincible, true),
        InvincibleStartDelay,
        false
    );

    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_InvincibleEnd,
        FTimerDelegate::CreateUObject(this, &UPlayerCombatComponent::SetInvincible, false),
        InvincibleStartDelay + InvincibleDuration,
        false
    );

    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_DodgeEnd,
        this,
        &UPlayerCombatComponent::FinishDodge,
        DodgeDuration,
        false
    );
}

void UPlayerCombatComponent::SetInvincible(bool bNewInvincible)
{
    bIsInvincible = bNewInvincible;

    UE_LOG(LogTemp, Warning, TEXT("[Combat] Invincible: %s"),
        bIsInvincible ? TEXT("true") : TEXT("false"));
}

void UPlayerCombatComponent::FinishDodge()
{
    bIsDodging = false;
    bIsInvincible = false;

    UE_LOG(LogTemp, Warning, TEXT("[Combat] Dodge finished"));
}

void UPlayerCombatComponent::Parry()
{
    /*if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry failed: OwnerCharacter is null"));
        return;
    }

    if (!OwnerStatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry failed: OwnerStatComponent is null"));
        return;
    }

    if (bIsAttacking)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry failed: currently attacking"));
        return;
    }

    if (bIsDodging)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry failed: currently dodging"));
        return;
    }

    if (bIsParrying)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry failed: already parrying"));
        return;
    }

    if (!OwnerStatComponent->HasEnoughStamina(ParryStaminaCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry failed: not enough stamina"));
        return;
    }*/

    if (!CanParry())
    {
        return;
    }

    OwnerStatComponent->ConsumeStamina(ParryStaminaCost);

    bIsParrying = true;
    bIsParryWindowOpen = false;

    if (ParryMontage)
    {
        OwnerCharacter->PlayAnimMontage(ParryMontage);
    }

    UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry started"));

    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_ParryWindowStart,
        this,
        &UPlayerCombatComponent::OpenParryWindow,
        ParryWindowStartDelay,
        false
    );

    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_ParryWindowEnd,
        this,
        &UPlayerCombatComponent::CloseParryWindow,
        ParryWindowStartDelay + ParryWindowDuration,
        false
    );

    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_ParryEnd,
        this,
        &UPlayerCombatComponent::FinishParry,
        ParryTotalDuration,
        false
    );
}

void UPlayerCombatComponent::OpenParryWindow()
{
    bIsParryWindowOpen = true;

    UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry window open"));
}

void UPlayerCombatComponent::CloseParryWindow()
{
    bIsParryWindowOpen = false;

    UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry window closed"));
}

void UPlayerCombatComponent::FinishParry()
{
    bIsParrying = false;
    bIsParryWindowOpen = false;

    UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry finished"));
}

bool UPlayerCombatComponent::CanAttack() const
{
    /*if (!OwnerCharacter)
    {
        return false;
    }

    if (!LightAttackMontage)
    {
        return false;
    }

    if (bIsAttacking || bIsDodging || bIsParrying)
    {
        return false;
    }

    if (OwnerStatComponent && OwnerStatComponent->IsDead())
    {
        return false;
    }

    return true;*/

    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] OwnerCharacter is null"));
        return false;
    }

    if (bIsAttacking)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Already attacking"));
        return false;
    }

    if (!LightAttackMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] LightAttackMontage is null"));
        return false;
    }

    return true;
}

bool UPlayerCombatComponent::CanDodge() const
{
    /*if (!OwnerCharacter || !OwnerStatComponent)
    {
        return false;
    }

    if (OwnerStatComponent->IsDead())
    {
        return false;
    }

    if (bIsAttacking || bIsDodging || bIsParrying)
    {
        return false;
    }

    return OwnerStatComponent->HasEnoughStamina(DodgeStaminaCost);*/

    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Dodge failed: OwnerCharacter is null"));
        return false;
    }

    if (!OwnerStatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Dodge failed: OwnerStatComponent is null"));
        return false;
    }

    if (bIsAttacking)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Dodge failed: currently attacking"));
        return false;
    }

    if (bIsDodging)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Dodge failed: already dodging"));
        return false;
    }

    if (!OwnerStatComponent->HasEnoughStamina(DodgeStaminaCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Dodge failed: not enough stamina"));
        return false;
    }

    return true;
}

bool UPlayerCombatComponent::CanParry() const
{
    /*if (!OwnerCharacter || !OwnerStatComponent)
    {
        return false;
    }

    if (OwnerStatComponent->IsDead())
    {
        return false;
    }

    if (bIsAttacking || bIsDodging || bIsParrying)
    {
        return false;
    }

    return OwnerStatComponent->HasEnoughStamina(ParryStaminaCost);*/

    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry failed: OwnerCharacter is null"));
        return false;
    }

    if (!OwnerStatComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry failed: OwnerStatComponent is null"));
        return false;
    }

    if (bIsAttacking)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry failed: currently attacking"));
        return false;
    }

    if (bIsDodging)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry failed: currently dodging"));
        return false;
    }

    if (bIsParrying)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry failed: already parrying"));
        return false;
    }

    if (!OwnerStatComponent->HasEnoughStamina(ParryStaminaCost))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Combat] Parry failed: not enough stamina"));
        return false;
    }

    return true;
}

void UPlayerCombatComponent::AddAttackDamage(float Amount)
{
    if (Amount <= 0.0f)
    {
        return;
    }

    AttackDamage += Amount;

    UE_LOG(LogTemp, Warning, TEXT("[CombatUpgrade] AddAttackDamage %.1f -> LightAttackDamage %.1f"),
        Amount, AttackDamage);
}

void UPlayerCombatComponent::ReduceDodgeStaminaCost(float Amount)
{
    if (Amount <= 0.0f)
    {
        return;
    }

    DodgeStaminaCost = FMath::Max(0.0f, DodgeStaminaCost - Amount);

    UE_LOG(LogTemp, Warning, TEXT("[CombatUpgrade] ReduceDodgeStaminaCost %.1f -> DodgeStaminaCost %.1f"),
        Amount, DodgeStaminaCost);
}

void UPlayerCombatComponent::AddPlayerParryPostureDamage(float Amount)
{
    if (Amount <= 0.0f)
    {
        return;
    }

    PlayerParryPostureDamage += Amount;

    UE_LOG(LogTemp, Warning, TEXT("[CombatUpgrade] AddPlayerParryPostureDamage %.1f -> PlayerParryPostureDamage %.1f"),
        Amount, PlayerParryPostureDamage);
}

float UPlayerCombatComponent::GetLightAttackDamage() const
{
    return AttackDamage;
}

float UPlayerCombatComponent::GetDodgeStaminaCost() const
{
    return DodgeStaminaCost;
}

float UPlayerCombatComponent::GetPlayerParryPostureDamage() const
{
    return PlayerParryPostureDamage;
}

void UPlayerCombatComponent::ResetCombatState()
{
    bIsAttacking = false;
    bAttackHitAlready = false;

    bIsDodging = false;
    bIsInvincible = false;

    bIsParrying = false;
    bIsParryWindowOpen = false;

    UE_LOG(LogTemp, Warning, TEXT("[Combat] ResetCombatState"));
}