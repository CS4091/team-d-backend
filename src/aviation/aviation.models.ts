import { ApiProperty } from '@nestjs/swagger';
import { fi } from 'src/utils/utils';

export interface RawRunway {
	id: string;
	airport_ref: string;
	airport_ident: string;
	length_ft: string;
	width_ft: string;
	surface: string;
	lighted: string;
	closed: string;
	le_ident: string;
}

export interface RawAirport {
	id: string;
	ident: string;
	type: string;
	name: string;
	latitude_deg: string;
	longitude_deg: string;
	elevation_ft: string;
	continent: string;
	iso_country: string;
	iso_region: string;
	municipality: string;
	scheduled_service: string;
	gps_code: string;
	iata_code: string;
	local_code: string;
}

export class Runway {
	@ApiProperty()
	name: string = fi();

	@ApiProperty()
	length: number = fi();

	@ApiProperty()
	width: number = fi();

	@ApiProperty()
	lighted: boolean = fi();
}

export class Airport {
	@ApiProperty()
	name: string = fi();

	@ApiProperty()
	city: string = fi();

	@ApiProperty()
	id: string = fi();

	@ApiProperty()
	iata: string = fi();

	@ApiProperty()
	lat: number = fi();

	@ApiProperty()
	lng: number = fi();

	@ApiProperty({ type: Runway })
	runways: Runway[] = fi();
}

export class City {
	@ApiProperty()
	name: string = fi();

	@ApiProperty()
	lat: number = fi();

	@ApiProperty()
	lng: number = fi();
}

export interface RawPlane {
	model: string;
	mtow: string;
	to_runway_len: string;
	land_runway_len: string;
	c5kft_as: string;
	cfl150_as: string;
	cfl240_as: string;
	cruise_as: string;
	dfl240_as: string;
	dfl100_as: string;
	approach_as: string;
}

export class PlaneModel {
	@ApiProperty()
	model: string = fi();

	@ApiProperty()
	mtow: number = fi();

	@ApiProperty()
	takeoffRunway: number = fi();

	@ApiProperty()
	landingRunway: number = fi();

	@ApiProperty()
	climb5kAirspeed: number = fi();

	@ApiProperty()
	climb15kAirspeed: number = fi();

	@ApiProperty()
	climb24kAirspeed: number = fi();

	@ApiProperty()
	cruiseAirspeed: number = fi();

	@ApiProperty()
	desc24kAirspeed: number = fi();

	@ApiProperty()
	desc10kAirspeed: number = fi();

	@ApiProperty()
	approachAirspeed: number = fi();
}

