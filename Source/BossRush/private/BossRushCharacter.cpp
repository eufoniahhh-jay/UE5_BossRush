// Copyright Epic Games, Inc. All Rights Reserved.

#include "BossRushCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "PlayerCombatComponent.h"
#include "PlayerStatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "BossBase.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ABossRushCharacter

ABossRushCharacter::ABossRushCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// Day2. 전투 컴포넌트
	CombatComponent = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("CombatComponent"));
	// Day3. 플레이어 스탯 컴포넌트
	StatComponent = CreateDefaultSubobject<UPlayerStatComponent>(TEXT("StatComponent"));

	/*UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Player Constructor] Combat=%s, Stat=%s"),
		*GetNameSafe(CombatComponent),
		*GetNameSafe(StatComponent)
	);*/
}

void ABossRushCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	/*UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Player BeginPlay] Combat=%s, Stat=%s"),
		*GetNameSafe(CombatComponent),
		*GetNameSafe(StatComponent)
	);*/
}

void ABossRushCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateLockOn(DeltaTime);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ABossRushCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABossRushCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABossRushCharacter::Look);

		// Day1. Attacking
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &ThisClass::LightAttack);

		// Day3. 플레이어 HP가 HUD 동기화 잘 되는지 확인 용도의 테스트
		EnhancedInputComponent->BindAction(TestDamageAction, ETriggerEvent::Started, this, &ThisClass::TestDamage);

		// Day5. 회피 액션 바인딩
		EnhancedInputComponent->BindAction(DodgeAction,	ETriggerEvent::Started,	this, &ThisClass::Dodge);

		// Day6. 패링 액션 바인딩
		EnhancedInputComponent->BindAction(ParryAction,	ETriggerEvent::Started,	this, &ThisClass::Parry);

		// Day7. 락온 액션 바인딩
		EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &ThisClass::ToggleLockOn);

		// Day14. 재시작 액션 바인딩
		if (RestartAction) {
			EnhancedInputComponent->BindAction(RestartAction, ETriggerEvent::Started, this, &ABossRushCharacter::Input_Restart);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ABossRushCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ABossRushCharacter::Look(const FInputActionValue& Value)
{
	// Day7. 락온 중 마우스 시점 조작 불가
	if (bIsLockOn)
	{
		return;
	}

	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ABossRushCharacter::LightAttack()
{
	// Day14. 게임 종료라면 인풋 막기
	if (bIsPlayerInputLocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Player] LightAttack input pressed but, bIsPlayerInputLocked"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Player] LightAttack input pressed"));

	/*if (!LightAttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Player] LightAttackMontage is null"));
		return;
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Player] AnimInstance is null"));
		return;
	}

	const float PlayedLength = AnimInstance->Montage_Play(LightAttackMontage, 1.0f);

	if (PlayedLength <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Player] Failed to play LightAttackMontage"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Player] Play LightAttackMontage: %s"), *LightAttackMontage->GetName());*/

	// Day2. 전투 컴포넌트
	// 이제 Character는 입력만 받고, 실제 공격 처리는 PlayerCombatComponent가 담당
	if (!CombatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Player] CombatComponent is null"));
		return;
	}

	CombatComponent->LightAttack();
}

void ABossRushCharacter::TestDamage()
{
	UE_LOG(LogTemp, Warning, TEXT("[Player] TestDamage input pressed"));

	if (!StatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Player] StatComponent is null"));
		return;
	}

	StatComponent->TakeDamage(10.0f);
}

void ABossRushCharacter::Dodge()
{
	// Day14. 게임 종료라면 인풋 막기
	if (bIsPlayerInputLocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Player] Dodge input pressed but, bIsPlayerInputLocked"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Player] Dodge input pressed"));

	if (!CombatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Player] CombatComponent is null"));
		return;
	}

	CombatComponent->Dodge();
}

void ABossRushCharacter::Parry()
{
	// Day14. 게임 종료라면 인풋 막기
	if (bIsPlayerInputLocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Player] Parry input pressed but, bIsPlayerInputLocked"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Player] Parry input pressed"));

	if (!CombatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Player] CombatComponent is null"));
		return;
	}

	CombatComponent->Parry();
}

void ABossRushCharacter::ToggleLockOn()
{
	// Day14. 게임 종료라면 인풋 막기
	if (bIsPlayerInputLocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LockOn] Toggle input pressed but, bIsPlayerInputLocked"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[LockOn] Toggle input pressed"));

	if (bIsLockOn)
	{
		DisableLockOn();
		return;
	}

	AActor* Target = FindLockOnTarget();

	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LockOn] No target found"));
		return;
	}

	EnableLockOn(Target);
}

void ABossRushCharacter::EnableLockOn(AActor* NewTarget)
{
	if (!NewTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LockOn] Enable failed: target is null"));
		return;
	}

	LockOnTarget = NewTarget;
	bIsLockOn = true;

	UE_LOG(LogTemp, Warning, TEXT("[LockOn] Enabled: %s"), *LockOnTarget->GetName());
}

void ABossRushCharacter::DisableLockOn()
{
	UE_LOG(LogTemp, Warning, TEXT("[LockOn] Disabled"));

	bIsLockOn = false;
	LockOnTarget = nullptr;
}

void ABossRushCharacter::UpdateLockOn(float DeltaTime)
{
	if (!bIsLockOn)
	{
		return;
	}

	if (!LockOnTarget)
	{
		DisableLockOn();
		return;
	}

	const float Distance = FVector::Distance(GetActorLocation(), LockOnTarget->GetActorLocation());

	if (Distance > LockOnMaxDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LockOn] Target too far. Disable lock on."));
		DisableLockOn();
		return;
	}

	const FVector MyLocation = GetActorLocation();
	const FVector TargetLocation = LockOnTarget->GetActorLocation();

	FRotator LookAtRotation = (TargetLocation - MyLocation).Rotation();

	// 캐릭터가 위아래로 기울지 않게 Yaw만 사용
	FRotator TargetYawRotation(0.0f, LookAtRotation.Yaw, 0.0f);

	FRotator NewActorRotation = FMath::RInterpTo(
		GetActorRotation(),
		TargetYawRotation,
		DeltaTime,
		LockOnInterpSpeed
	);

	SetActorRotation(NewActorRotation);

	AController* OwnerController = GetController();

	if (OwnerController)
	{
		/*const FVector CameraFocusLocation = LockOnTarget->GetActorLocation() + FVector(0.0f, 0.0f, 80.0f);
		const FVector CameraStartLocation = GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);*/
		const FVector CameraFocusLocation = LockOnTarget->GetActorLocation() + FVector(0.0f, 0.0f, 80.0f);
		const FVector CameraStartLocation = GetActorLocation() + FVector(0.0f, 0.0f, 140.0f);

		FRotator TargetControlRotation = (CameraFocusLocation - CameraStartLocation).Rotation();

		FRotator NewControlRotation = FMath::RInterpTo(
			OwnerController->GetControlRotation(),
			TargetControlRotation,
			DeltaTime,
			LockOnInterpSpeed
		);

		OwnerController->SetControlRotation(NewControlRotation);
	}
}

AActor* ABossRushCharacter::FindLockOnTarget() const
{
	TArray<AActor*> BossActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Boss"), BossActors);

	if (BossActors.Num() == 0)
	{
		return nullptr;
	}

	AActor* BestTarget = nullptr;
	float BestDistanceSq = LockOnMaxDistance * LockOnMaxDistance;

	const FVector MyLocation = GetActorLocation();

	for (AActor* Candidate : BossActors)
	{
		if (!Candidate)
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(MyLocation, Candidate->GetActorLocation());

		if (DistanceSq <= BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

bool ABossRushCharacter::CanRestartBattle() const
{
	if (StatComponent && StatComponent->IsDead())
	{
		return true;
	}

	ABossBase* Boss = Cast<ABossBase>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ABossBase::StaticClass())
	);

	if (Boss && Boss->IsDead())
	{
		return true;
	}

	return false;
}

void ABossRushCharacter::Input_Restart()
{
	if (!CanRestartBattle())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Restart] Restart ignored: battle is not ended"));
		return;
	}

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);

	UE_LOG(LogTemp, Warning, TEXT("[Restart] Restart level: %s"), *CurrentLevelName);

	UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
}

void ABossRushCharacter::SetPlayerInputLocked(bool bLocked)
{
	bIsPlayerInputLocked = bLocked;

	if (bIsPlayerInputLocked && GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
	}
}