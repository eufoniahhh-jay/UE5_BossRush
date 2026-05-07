// PlayerCombatComponent.cpp

#include "PlayerCombatComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

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