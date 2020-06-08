// Fill out your copyright notice in the Description page of Project Settings.


#include "TestDlibActor.h"

#include "dlib/svm.h"

#include "Core/Public/Async/Async.h"
#include "Misc/App.h"
#include "Misc/Paths.h"





#include <iostream>

using namespace std;
using namespace dlib;

// Sets default values
ATestDlibActor::ATestDlibActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATestDlibActor::BeginPlay()
{
	Super::BeginPlay();
	AsyncTask(ENamedThreads::AnyThread, [=]()
      {
            typedef matrix<double, 2, 1> sample_type;
			typedef radial_basis_kernel<sample_type> kernel_type;


			// Now we make objects to contain our samples and their respective labels.
			std::vector<sample_type> samples;
			std::vector<double> labels;

			// Now let's put some data into our samples and labels objects.  We do this
			// by looping over a bunch of points and labeling them according to their
			// distance from the origin.
			for (int r = -20; r <= 20; ++r)
			{
				for (int c = -20; c <= 20; ++c)
				{
					sample_type samp;
					samp(0) = r;
					samp(1) = c;
					samples.push_back(samp);

					// if this point is less than 10 from the origin
					if (sqrt((double)r*r + c*c) <= 10)
						labels.push_back(+1);
					else
						labels.push_back(-1);

				}
			}

			vector_normalizer<sample_type> normalizer;
			// Let the normalizer learn the mean and standard deviation of the samples.
			normalizer.train(samples);
			// now normalize each sample
			for (unsigned long i = 0; i < samples.size(); ++i)
				samples[i] = normalizer(samples[i]);

			randomize_samples(samples, labels);


			// here we make an instance of the svm_c_trainer object that uses our kernel
			// type.
			svm_c_trainer<kernel_type> trainer;

			UE_LOG(LogTemp, Warning, TEXT("doing cross validation"));
			UE_LOG(LogTemp, Warning, TEXT("doing cross validation   dfdfadfadfadfasdfasdfadsfasdfasdfasdfasdfasdfasdf"));
			//cout << "doing cross validation" << endl;
			std::stringstream strout;
			for (double gamma = 0.00001; gamma <= 1; gamma *= 5)
			{
				for (double C = 1; C < 100000; C *= 5)
				{
					// tell the trainer the parameters we want to use
					trainer.set_kernel(kernel_type(gamma));
					trainer.set_c(C);

					//UE_LOG(LogTemp, Warning, TEXT(""), gamma, C);
					cout << "gamma: " << gamma << "    C: " << C;
					// Print out the cross validation accuracy for 3-fold cross validation using
					// the current gamma and C.  cross_validate_trainer() returns a row vector.
					// The first element of the vector is the fraction of +1 training examples
					// correctly classified and the second number is the fraction of -1 training
					// examples correctly classified.
					cout << "     cross validation accuracy: "
						<< cross_validate_trainer(trainer, samples, labels, 3);
					//const matrix<double, 1, 2, typename trainer_type::mem_manager_type> test;
					strout.str("");
					strout << cross_validate_trainer(trainer, samples, labels, 3);
					
					UE_LOG(LogTemp, Warning, TEXT(" gamma: %f   C:  %f   cross validation accuracy: %s"), gamma, C, UTF8_TO_TCHAR(strout.str().c_str()));
				}
			}

			trainer.set_kernel(kernel_type(0.15625));
			trainer.set_c(5);
			typedef decision_function<kernel_type> dec_funct_type;
			typedef normalized_function<dec_funct_type> funct_type;

			// Here we are making an instance of the normalized_function object.  This
			// object provides a convenient way to store the vector normalization
			// information along with the decision function we are going to learn.  
			funct_type learned_function;
			learned_function.normalizer = normalizer;  // save normalization information
			learned_function.function = trainer.train(samples, labels); // perform the actual SVM training and save the results

																		// print out the number of support vectors in the resulting decision function
			cout << "\nnumber of support vectors in our learned_function is "
				<< learned_function.function.basis_vectors.size() << endl;

			UE_LOG(LogTemp, Warning, TEXT("number of support vectors in our learned_function is %i"), learned_function.function.basis_vectors.size());

			std::stringstream strOut;
			// Now let's try this decision_function on some samples we haven't seen before.
			sample_type sample;
			
			sample(0) = 3.123;
			sample(1) = 2;
			cout << "This is a +1 class example, the classifier output is " << learned_function(sample) << endl;
			strOut.str("");
			strOut << learned_function(sample);
			UE_LOG(LogTemp, Warning, TEXT(" This is a +1 class example, the classifier output is %s"), UTF8_TO_TCHAR(strOut.str().c_str()));

			sample(0) = 3.123;
			sample(1) = 9.3545;
			cout << "This is a +1 class example, the classifier output is " << learned_function(sample) << endl;
			strOut.str("");
			strOut << learned_function(sample);
			UE_LOG(LogTemp, Warning, TEXT(" This is a +1 class example, the classifier output is %s"), UTF8_TO_TCHAR(strOut.str().c_str()));

			sample(0) = 13.123;
			sample(1) = 9.3545;
			cout << "This is a -1 class example, the classifier output is " << learned_function(sample) << endl;
			strOut.str("");
			strOut << learned_function(sample);
			UE_LOG(LogTemp, Warning, TEXT(" This is a +1 class example, the classifier output is %s"), UTF8_TO_TCHAR(strOut.str().c_str()));

			sample(0) = 13.123;
			sample(1) = 0;
			cout << "This is a -1 class example, the classifier output is " << learned_function(sample) << endl;
			strOut.str("");
			strOut << learned_function(sample);
			UE_LOG(LogTemp, Warning, TEXT(" This is a +1 class example, the classifier output is %s"), UTF8_TO_TCHAR(strOut.str().c_str()));


			// We can also train a decision function that reports a well conditioned
			// probability instead of just a number > 0 for the +1 class and < 0 for the
			// -1 class.  An example of doing that follows:
			typedef probabilistic_decision_function<kernel_type> probabilistic_funct_type;
			typedef normalized_function<probabilistic_funct_type> pfunct_type;

			pfunct_type learned_pfunct;
			learned_pfunct.normalizer = normalizer;
			learned_pfunct.function = train_probabilistic_decision_function(trainer, samples, labels, 3);
			// Now we have a function that returns the probability that a given sample is of the +1 class.  

			// print out the number of support vectors in the resulting decision function.  
			// (it should be the same as in the one above)
			cout << "\nnumber of support vectors in our learned_pfunct is "
				<< learned_pfunct.function.decision_funct.basis_vectors.size() << endl;
			UE_LOG(LogTemp, Warning, TEXT("number of support vectors in our learned_pfunct is %i"), learned_pfunct.function.decision_funct.basis_vectors.size());

			sample(0) = 3.123;
			sample(1) = 2;
			cout << "This +1 class example should have high probability.  Its probability is: "
				<< learned_pfunct(sample) << endl;
			strOut.str("");
			strOut << learned_pfunct(sample);
			UE_LOG(LogTemp, Warning, TEXT(" This +1 class example should have high probability.  Its probability is:  %s"), UTF8_TO_TCHAR(strOut.str().c_str()));

			sample(0) = 3.123;
			sample(1) = 9.3545;
			cout << "This +1 class example should have high probability.  Its probability is: "
				<< learned_pfunct(sample) << endl;
			strOut.str("");
			strOut << learned_pfunct(sample);
			UE_LOG(LogTemp, Warning, TEXT(" This +1 class example should have high probability.  Its probability is:  %s"), UTF8_TO_TCHAR(strOut.str().c_str()));

			sample(0) = 13.123;
			sample(1) = 9.3545;
			cout << "This -1 class example should have low probability.  Its probability is: "
				<< learned_pfunct(sample) << endl;
			strOut.str("");
			strOut << learned_pfunct(sample);
			UE_LOG(LogTemp, Warning, TEXT(" This +1 class example should have high probability.  Its probability is:  %s"), UTF8_TO_TCHAR(strOut.str().c_str()));

			sample(0) = 13.123;
			sample(1) = 0;
			cout << "This -1 class example should have low probability.  Its probability is: "
				<< learned_pfunct(sample) << endl;
			strOut.str("");
			strOut << learned_pfunct(sample);
			UE_LOG(LogTemp, Warning, TEXT(" This +1 class example should have high probability.  Its probability is:  %s"), UTF8_TO_TCHAR(strOut.str().c_str()));

			//for android path bug
			FString TempPath;
			TempPath = FPaths::Combine(FPaths::GameContentDir(), TEXT("TestRes/saved_function.dat"));
#if PLATFORM_ANDROID
			extern const FString &GetFileBasePath();
	        FString ProjectPath = GetFileBasePath();
	        TempPath = FPaths::Combine(ProjectPath, FApp::GetProjectName(), TEXT("Content/TestRes/saved_function.dat"));
			//TempPath = TEXT("/storage/emulated/0/saved_function.dat");
#endif

			UE_LOG(LogTemp, Warning, TEXT("Begin Serialize file TempPath = %s"), *TempPath );
			std::string strTemp(TCHAR_TO_UTF8(*TempPath));
			std::ofstream StreamOut(strTemp.c_str(), std::ios::binary);
			
			UE_LOG(LogTemp, Warning, TEXT("custom serialize begin, TempPath = %s"), *TempPath );
			serialize(learned_pfunct, StreamOut);
			StreamOut.clear();
			StreamOut.close();
			UE_LOG(LogTemp, Warning, TEXT("custom serialize end, TempPath = %s"), *TempPath );

			std::ifstream StreamIn(strTemp.c_str(), std::ios::binary);
			UE_LOG(LogTemp, Warning, TEXT("custom deserialize begin, begin deserialize TempPath = %s"), *TempPath );
			deserialize(learned_pfunct, StreamIn);
			UE_LOG(LogTemp, Warning, TEXT("custom deserialize end, begin deserialize TempPath = %s"), *TempPath );

	
			// serialize(TCHAR_TO_UTF8(*TempPath)) << learned_pfunct;
			// UE_LOG(LogTemp, Warning, TEXT("serialize success, begin deserialize TempPath = %s"), *TempPath );
			// // Now let's open that file back up and load the function object it contains.
			// deserialize(TCHAR_TO_UTF8(*TempPath)) >> learned_pfunct;
			
			UE_LOG(LogTemp, Warning, TEXT("Begin Cross validatetrainer"));
			// cout << "\ncross validation accuracy with only 10 support vectors: "
			// 	<< cross_validate_trainer(reduced2(trainer, 10), samples, labels, 3);
			strOut.str("");
			strOut << cross_validate_trainer(reduced2(trainer, 10), samples, labels, 3);
			UE_LOG(LogTemp, Warning, TEXT("cross validation accuracy with only 10 support vectors:  %s"), UTF8_TO_TCHAR(strOut.str().c_str()));

			// Let's print out the original cross validation score too for comparison.
			// cout << "cross validation accuracy with all the original support vectors: "
			// 	<< cross_validate_trainer(trainer, samples, labels, 3);
			strOut.str("");
			// strOut << cross_validate_trainer(trainer, randomly_subsample(samples, 1), randomly_subsample(labels,  1), 3);
			// UE_LOG(LogTemp, Warning, TEXT("cross validation accuracy with all the original support vectors:  %s"), UTF8_TO_TCHAR(strOut.str().c_str()));

			// When you run this program you should see that, for this problem, you can
			// reduce the number of basis vectors down to 10 without hurting the cross
			// validation accuracy. 


			// To get the reduced decision function out we would just do this:
			learned_function.function = reduced2(trainer, 10).train(samples, labels);
			// And similarly for the probabilistic_decision_function: 
			learned_pfunct.function = train_probabilistic_decision_function(reduced2(trainer, 10), samples, labels, 3);
            return;
    });
	

}

// Called every frame
void ATestDlibActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}