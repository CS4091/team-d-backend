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

