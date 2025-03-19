import { parse } from '@fast-csv/parse';
import { Injectable } from '@nestjs/common';
import { createReadStream } from 'fs';
import { Airport, RawAirport, RawRunway } from './aviation.models';

@Injectable()
export class AviationService {
	public readonly airports: Promise<Airport[]>;

	public constructor() {
		this.airports = new Promise<Airport[]>((resolve, reject) => {
			const data: Airport[] = [];

			new Promise<void>((resolve, reject) =>
				createReadStream('data/airports.csv')
					.pipe(
						parse({
							objectMode: true,
							trim: true,
							headers: true
						})
					)
					.on('error', reject)
					.on(
						'data',
						({ type, ident, iata_code, latitude_deg, longitude_deg, name }: RawAirport) =>
							/^(small|medium|large)_airport$/.test(type) &&
							iata_code !== '' &&
							data.push({ name, id: ident, iata: iata_code, lat: Number(latitude_deg), lng: Number(longitude_deg), runways: [] })
					)
					.on('end', () => resolve())
			)
				.then(() =>
					createReadStream('data/runways.csv')
						.pipe(
							parse({
								objectMode: true,
								trim: true,
								headers: true
							})
						)
						.on('error', reject)
						.on('data', ({ airport_ident, le_ident, length_ft, width_ft, lighted }: RawRunway) =>
							data
								.find((airport) => airport.id === airport_ident)
								?.runways.push({ name: le_ident, length: Number(length_ft), width: Number(width_ft), lighted: lighted === '1' })
						)
						.on('end', () => resolve(data))
				)
				.catch((err) => reject(err));
		})
			.catch((err) => {
				console.error(err);
				throw new Error('Failed to acquire airports.csv or airports_intl.csv');
			})
			.then((airports) => airports.filter((airport) => airport.runways.length > 0));
	}
}

