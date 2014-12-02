#include "watcard_office.h"
#include "MPRNG.h"

using namespace std;

WATCardOffice::WATCardOffice( Printer &prt, Bank &bank, unsigned int numCouriers ) : _prt( prt ), _bank( bank ), 
                             _numCouriers( numCouriers ) 
{
	_studentMachineIdMap = new Courier[numCouriers];
	for ( unsigned int i = 0; i < _numCouriers; i++ ) {
		_couriers[i] = new WATCardOffice::Courier( i, _prt, _bank, *this );
	}
}

WATCardOffice::~WATCardOffice() {
	for ( unsigned int i = 0; i < _numCouriers; i++ ) {
		_couriers[i];
	}
	delete[] _couriers;
}

void WATCardOffice::main() {
	 _prt.print( Printer::WATCardOffice, WATCardOffice::Starting );
	
	for ( ;; ) {

		_Accept ( ~WATCardOffice ) {
			break;
		} or _Accept( create ) {
			_jobs.push( _currentJob ); // add job to queue then let the couriers know
			courierBench.signal();
		} or _Accept( transfer ) {
			_jobs.push( _currentJob );
			courierBench.signal();
		} 
			
	}
	_prt.print( Printer::WATCardOffice, WATCardOffice::Finished );
}

WATCard::FWATCard WATCardOffice::create( unsigned int sid, unsigned int amount ) {
	_currentJob = new Job(Args(sid, amount, NULL)); // this will only work is the _Accept( create ) is guaranteed to run right after
	
	return _currentJob->result; 
}

WATCard::FWATCard WATCardOffice::transfer( unsigned int sid, unsigned int amount, WATCard *card ) {
	_currentJob = new Job(Args(sid, amount, card));
	
	return _currentJob->result;
}

Job *WATCardOffice::requestWork() {
	Job* requestedJob = _jobs.front(); // should never be empty because of courierBench
	_jobs.pop();
	return requestedJob;
}

/*Courier Implementation*/

WATCardOffice::Courier::Courier( unsigned int id, Printer &prt, Bank &bank, WATCardOffice &watOffice ) : _id( id ), 
                                _prt( prt ), _bank( bank ), _watOffice( watOffice ) {
	
}

void WATCardOffice::Courier::main() {
	_prt.print( Printer::Courier, WATCardOffice::Courier::Starting );
	for ( ;; ) {
		_Accept ( ~Courier ) {
			break;
		} _Else { // continue with the rest of the loop
			
			courierBench.wait(); // when unblocked there should be a job
			
			_prt.print( Printer::Courier, WATCardOffice::Courier::TransferStart );
			
			Job* newJob = requestWork();
			
			if (newJob->args._card == NULL) { //create
				newJob->args._card == WATCard();
			}
			
			_bank.withdraw(newJob->args._sid, newJob->args._amount);
			newJob->args._card.deposit( newJob->args._sid, newJob->args._amount );
			
			if ( g_mprng(5) == 3 ) { // simulate 1 in 6 chance of losing card
				delete newJob->newJob->args._card;
				newJob->result.exception( new Lost() );
			} else {
				newJob->result.deliver( newJob->args._card );
			}
			
			_prt.print( Printer::Courier, WATCardOffice::Courier::TransferComplete );
			
			delete newJob;
        }
			
	}
	_prt.print( Printer::Courier, WATCardOffice::Courier::Finished );
}

