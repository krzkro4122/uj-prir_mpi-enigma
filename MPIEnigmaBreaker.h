/*
 * MPIEnigmaBreaker.h
 *
 *  Created on: 26 paź 2022
 *      Author: kr.krol@student.uj.edu.pl
 */

#ifndef MPIENIGMABREAKER_H_
#define MPIENIGMABREAKER_H_

#include"EnigmaBreaker.h"

class MPIEnigmaBreaker : public EnigmaBreaker {
private:
	bool solutionFound( uint *rotorSettingsProposal );
public:
	MPIEnigmaBreaker( Enigma *enigma, MessageComparator *comparator );

	void crackMessage();
	void getResult( uint *rotorPositions );
	
	virtual void setSampleToFind( uint *expected, uint expectedLength );
	virtual void setMessageToDecode( uint *message, uint messageLength );

	virtual ~MPIEnigmaBreaker();
};

#endif /* MPIENIGMABREAKER_H_ */