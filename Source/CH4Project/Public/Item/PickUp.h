#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUp.generated.h"

class UStaticMeshComponent;

UCLASS()
class CH4PROJECT_API APickUp : public AActor
{
	GENERATED_BODY()
	
public:	
	APickUp();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PickUp")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PickUp")
	TSubclassOf<class UBaseItem> ItemClass;
	
	virtual void BeginPlay() override;
	void Tick(float DeltaSeconds);

	UFUNCTION()
	void OnOverlap(AActor* OverlapActor, AActor* OtherActor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	UTexture2D* Icon;

private:
	FVector StartLocation;
	float RunningTime;

	UPROPERTY(EditDefaultsOnly, Category="Item|Effect")
	float RotationSpeed = 60.f;

	UPROPERTY(EditDefaultsOnly, Category="Item|Effect")
	float BobHeight = 20.f;

	UPROPERTY(EditDefaultsOnly, Category="Item|Effect")
	float BobSpeed = 2.f;

};
