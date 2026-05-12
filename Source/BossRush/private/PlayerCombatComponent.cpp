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
    if (!OwnerCharacter)
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
    if (!OwnerCharacter)
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