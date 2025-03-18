import { ApiProperty } from '@nestjs/swagger';
import { fi } from 'src/utils/utils';

export interface RawAirport {
	siteID: string;
	facilityType: 'AIRPORT';
	locID: string;
	effectiveDate: string;
	region: string;
	ado: string;
	stateID: string;
	stateName: string;
	county: string;
	countyState: string;
	city: string;
	name: string;
	ownership: string;
	use: string;
	owner: string;
	ownerAddress: string;
	ownerCity: string;
	ownerPhone: string;
	manager: string;
	managerAddress: string;
	managerCityStateZip: string;
	managerPhone: string;
	arpLatitude: string;
	arpLatitudeSec: string;
	arpLongitude: string;
	arpLongitudeSec: string;
	arpMethod: string;
	elevation: string;
	elevationMethod: string;
	magneticVariation: string;
	magneticVariationYear: string;
	trafficPatternAltitude: string;
	sectional: string;
	distanceFromCBD: string;
	directionFromCBD: string;
	landArea: string;
	artccID: string;
	artccComputerID: string;
	artccName: string;
	responsibleARTCCID: string;
	responsibleARTCCComputerID: string;
	responsibleARTCCName: string;
	tieInFSS: string;
	tieInFSSID: string;
	tieInFSSName: string;
	fssPhoneNumber: string;
	fssTollFreeNumber: string;
	alternateFSSID: string;
	alternateFSSName: string;
	alternateFSSTollFreeNumber: string;
	notamFacilityID: string;
	notamService: string;
	activationDate: string;
	airportStatusCode: string;
	certificationTypeDate: string;
	federalAgreements: string;
	airspaceDetermination: string;
	customsAirportOfEntry: string;
	customsLandingRights: string;
	militaryJointUse: string;
	militaryLandingRights: string;
	inspectionMethod: string;
	inspectionGroup: string;
	lastInspectionDate: string;
	lastOwnerInformationDate: string;
	fuelTypes: string;
	airframeRepair: string;
	powerPlantRepair: string;
	bottledOxygenType: string;
	bulkOxygenType: string;
	lightingSchedule: string;
	beaconSchedule: string;
	atct: string;
	unicom: string;
	ctaf: string;
	segmentedCircle: string;
	beaconColor: string;
	nonCommercialLandingFee: string;
	medicalUse: string;
	singleEngineAircraft: string;
	multiEngineAircraft: string;
	jetEngineAircraft: string;
	helicopters: string;
	glidersOperational: string;
	militaryOperational: string;
	ultralights: string;
	commercialOperations: string;
	commuterOperations: string;
	airTaxiOperations: string;
	gaLocalOperations: string;
	gaItinOperations: string;
	militaryOperations: string;
	operationsDate: string;
	airportPositionSource: string;
	airportPositionSourceDate: string;
	airportElevationSource: string;
	airportElevationSourceDate: string;
	fuelAvailable: string;
	transientStorage: string;
	otherServices: string;
	windIndicator: string;
	icaoID: string;
	npiasHub: string;
	npiasRole: string;
	arpLatitudeDD: string;
	arpLongitudeDD: string;
}

export interface RawIntlAirport {
	id: string;
	ICAO: string;
	type: string;
	name: string;
	lat: string;
	lng: string;
	unknown1: string;
	region: string;
	country: string;
	state: string;
	short: string;
	unknown2: string;
	ICAO2: string;
	IATA: string;
	ICAO3: string;
	unknown3: string;
	website: string;
	wikipedia: string;
	unknown4: string;
}

export class Airport {
	@ApiProperty()
	name: string = fi();

	@ApiProperty()
	id: string = fi();

	@ApiProperty()
	lat: number = fi();

	@ApiProperty()
	lng: number = fi();
}

