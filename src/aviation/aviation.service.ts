import { parse } from '@fast-csv/parse';
import { Injectable } from '@nestjs/common';
import { createReadStream } from 'fs';
import { Airport, RawAirport, RawIntlAirport } from './aviation.models';
import { processHeaders } from './utils';

@Injectable()
export class AviationService {
	public readonly airports: Promise<Airport[]>;

	public constructor() {
		this.airports = new Promise<Airport[]>((resolve, reject) => {
			const data: Airport[] = [];

			createReadStream('data/airports.csv')
				.pipe(
					parse({
						objectMode: true,
						trim: true,
						headers: processHeaders
					})
				)
				.on('error', reject)
				.on(
					'data',
					({ facilityType, name, icaoID, arpLatitudeDD, arpLongitudeDD }: RawAirport) =>
						facilityType === 'AIRPORT' && icaoID !== '' && data.push({ name, id: icaoID, lat: Number(arpLatitudeDD), lng: Number(arpLongitudeDD) })
				)
				.on('end', () => resolve(data));

			createReadStream('data/airports_intl.csv')
				.pipe(
					parse({
						objectMode: true,
						trim: true,
						headers: processHeaders
					})
				)
				.on('error', reject)
				.on(
					'data',
					({ name, ICAO, lat, lng, country }: RawIntlAirport) =>
						country !== 'US' && data.push({ name, id: ICAO, lat: Number(lat), lng: Number(lng) })
				)
				.on('end', () => resolve(data));
		}).catch((err) => {
			console.error(err);
			throw new Error('Failed to acquire airports.csv or airports_intl.csv');
		});
	}
}

