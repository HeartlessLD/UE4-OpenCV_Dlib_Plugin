// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
// #pragma warning(disable: 4458)
// #pragma warning(disable: 4459)
// #pragma warning(disable: 4456)



#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestDlibActor.generated.h"

UCLASS()
class DLIB_API ATestDlibActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestDlibActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
