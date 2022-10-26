/*
 * MPIEnigmaBreaker.cpp
 *
 *  Created on: 26 paź 2022
 *      Author: kr.krol@student.uj.edu.pl
 */

#include "MPIEnigmaBreaker.h"
#include "mpi.h"

MPIEnigmaBreaker::MPIEnigmaBreaker( Enigma *enigma, MessageComparator *comparator ) : EnigmaBreaker(enigma, comparator ) {
}

MPIEnigmaBreaker::~MPIEnigmaBreaker() {
	delete[] rotorPositions;
}

void MPIEnigmaBreaker::crackMessage() {
	uint rotorLargestSetting = enigma->getLargestRotorSetting();

	int rank;
	int size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	cout << "[START] Process " << to_string(rank) << endl;

	double timeStart = MPI_Wtime();

	// for (int i = rank; i < 4; i+=size) {
	// 	for (int j = 0; j < 4; j+=1) {
	// 		cout << to_string(rank) << " says (" << to_string(i) << ", " << to_string(j) << ")" << endl;
	// 	}
	// }

	/**
	 * Poniższy kod (w szczególności pętle) jest paskudny. Można to
	 * napisac ładniej i, przede wszystkim w wersji uniwersalnej,
	 * ale szkoda czasu. Ta proteza tu wystarczy.
	 * Umawiamy się niniejszym, że więcej niż maxRotors rotorów nie będzie!
	 */

	uint *rMax = new uint[ MAX_ROTORS ];
	for ( uint rotor = 0; rotor < MAX_ROTORS; rotor++ ) {
		if ( rotor < rotors )
			rMax[ rotor ] = rotorLargestSetting;
		else
			rMax[ rotor ] = 0;
	}

	uint *r = new uint[ MAX_ROTORS ];

	for ( r[0] = rank; r[0] <= rMax[0]; r[0] += size )  // This spreads the compute load to all processes EVENLY.
		for ( r[1] = 0; r[1] <= rMax[1]; r[1]++ )
			for ( r[2] = 0; r[2] <= rMax[2]; r[2]++ )
				for ( r[3] = 0; r[3] <= rMax[3]; r[3]++ )
					for ( r[4] = 0; r[4] <= rMax[4]; r[4]++ )
						for ( r[5] = 0; r[5] <= rMax[5]; r[5]++ )
							for ( r[6] = 0; r[6] <= rMax[6]; r[6]++ )
								for ( r[7] = 0; r[7] <= rMax[7]; r[7]++ )
									for ( r[8] = 0; r[8] <= rMax[8]; r[8]++ )
										for ( r[9] = 0; r[9] <= rMax[9]; r[9]++ ) {
											if ( solutionFound( r ) )
												goto EXIT_ALL_LOOPS;
										}
	EXIT_ALL_LOOPS:

	if (rank == 0) {
        double timeEnd = MPI_Wtime();
		cout << "Czas obliczeń: " << to_string(timeEnd - timeStart) << " s" << endl;
	}

	delete[] rMax;
	delete[] r;
}

bool MPIEnigmaBreaker::solutionFound( uint *rotorSettingsProposal ) {
	for ( uint rotor = 0; rotor < rotors; rotor++ )
		rotorPositions[ rotor ] = rotorSettingsProposal[ rotor ];

	enigma->setRotorPositions(rotorPositions);
	uint *decodedMessage = new uint[ messageLength ];

	for (uint messagePosition = 0; messagePosition < messageLength; messagePosition++ ) {
		decodedMessage[ messagePosition ] = enigma->code(messageToDecode[ messagePosition ] );
	}

	bool result = comparator->messageDecoded(decodedMessage);

	delete[] decodedMessage;

	return result;
}

void MPIEnigmaBreaker::getResult( uint *rotorPositions ) {
	for ( uint rotor = 0; rotor < rotors; rotor++ ) {
		rotorPositions[ rotor ] = this->rotorPositions[ rotor ];
	}
}

void MPIEnigmaBreaker::setMessageToDecode( uint *message, uint messageLength ) {
	comparator->setMessageLength(messageLength);
	this->messageLength = messageLength;
	this->messageToDecode = message;
	messageProposal = new uint[ messageLength ];
}

void MPIEnigmaBreaker::setSampleToFind(uint *expected, uint expectedLength ) {
	comparator->setExpectedFragment(expected, expectedLength);
}
