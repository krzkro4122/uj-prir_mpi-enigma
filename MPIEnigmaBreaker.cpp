/*
 * MPIEnigmaBreaker.cpp
 *
 *  Created on: 26 paź 2022
 *      Author: kr.krol@student.uj.edu.pl
 */

#include "MPIEnigmaBreaker.h"
#include "mpi.h"

MPIEnigmaBreaker::MPIEnigmaBreaker( Enigma *enigma, MessageComparator *comparator ) : EnigmaBreaker(enigma, comparator ) {
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
}

MPIEnigmaBreaker::~MPIEnigmaBreaker() {
	delete[] rotorPositions;
}

void MPIEnigmaBreaker::crackMessage() {

	// --- Communication ---
	if (size > 1){
		// Message lengths
		MPI_Bcast(&messageLength, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
		MPI_Bcast(&expectedLength, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
		if (rank != 0) {
			expected = new uint[expectedLength];
			messageToDecode = new uint[messageLength];
		}
		// Messages
		MPI_Bcast(messageToDecode, messageLength, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
		MPI_Bcast(expected, expectedLength, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
		// Configure the comparator
		comparator->setMessageLength(messageLength);
		comparator->setExpectedFragment(expected, expectedLength);
	}

	uint rotorLargestSetting = enigma->getLargestRotorSetting();

	// Now scatter it or sth
	uint maxRange = MAX_ROTORS * 2;
	// maxRange ustawien - suma czasow wszystkich procesow
	// x ustawien - czas poszczegolnego procesu

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

	// Init the needed variables for communication
	int finder = -1;
	int ready = 0;
	MPI_Request request;
	MPI_Status status;

	// Receive&go
	MPI_Irecv(&finder, 1, MPI_UNSIGNED, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &request);

	// The first line spreads the compute load to all processes EVENLY.
	for ( r[0] = rank; r[0] <= rMax[0] - size; r[0] += size )
		for ( r[1] = 0; r[1] <= rMax[1]; r[1]++ )
			for ( r[2] = 0; r[2] <= rMax[2]; r[2]++ )
				for ( r[3] = 0; r[3] <= rMax[3]; r[3]++ )
					for ( r[4] = 0; r[4] <= rMax[4]; r[4]++ )
						for ( r[5] = 0; r[5] <= rMax[5]; r[5]++ )
							for ( r[6] = 0; r[6] <= rMax[6]; r[6]++ )
								for ( r[7] = 0; r[7] <= rMax[7]; r[7]++ )
									for ( r[8] = 0; r[8] <= rMax[8]; r[8]++ )
										for ( r[9] = 0; r[9] <= rMax[9]; r[9]++ ) {
											// See if anybody found the answer already
											MPI_Test(&request, &ready, &status);
											if ( ready ) {
												goto EXIT_ALL_LOOPS;
											}
											// When the answer is found - send it to all other processes
											if ( solutionFound( r ) ) {
												finder = rank;
												for (int i = 0; i < size; i++) {
													if (i != finder)
														MPI_Isend(&finder, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD, &request);
												}
												goto EXIT_ALL_LOOPS;
											}
										}
	EXIT_ALL_LOOPS:

	// If finder was non-root, send the correct answer to root and use it
	if (finder != 0 && rank == 0) {
		ready = 0;
		MPI_Recv(r, MAX_ROTORS, MPI_UNSIGNED, finder, 0, MPI_COMM_WORLD, &status);
		solutionFound( r );  // use
	}
	if (finder == rank && finder != 0)
		MPI_Isend(r, MAX_ROTORS, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD, &request);

	delete[] rMax;
	delete[] r;
}

bool MPIEnigmaBreaker::solutionFound( uint *rotorSettingsProposal ) {
	for ( uint rotor = 0; rotor < rotors; rotor++ )
		rotorPositions[ rotor ] = rotorSettingsProposal[ rotor ];

	enigma->setRotorPositions(rotorPositions);
	uint *decodedMessage = new uint[ messageLength ];

	for (uint messagePosition = 0; messagePosition < messageLength; messagePosition++ )
		decodedMessage[ messagePosition ] = enigma->code(messageToDecode[ messagePosition ] );

	bool result = comparator->messageDecoded(decodedMessage);

	delete[] decodedMessage;

	return result;
}

void MPIEnigmaBreaker::getResult( uint *rotorPositions ) {
	for ( uint rotor = 0; rotor < rotors; rotor++ )
		rotorPositions[ rotor ] = this->rotorPositions[ rotor ];
}

void MPIEnigmaBreaker::setSampleToFind( uint *expected, uint expectedLength ) {
	EnigmaBreaker::setSampleToFind(expected, expectedLength);
	this->expected = expected;
	this->expectedLength = expectedLength;
}