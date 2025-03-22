import { parse } from '@fast-csv/parse';
import { Injectable } from '@nestjs/common';
import { createReadStream } from 'fs';
import { Airport, City, PlaneModel, RawAirport, RawPlane, RawRunway } from './aviation.models';

@Injectable()
export class AviationService {
	public readonly airports: Promise<Airport[]>;
	public readonly cities: Promise<City[]>;
	public readonly planes: Promise<PlaneModel[]>;

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
						({ type, ident, iata_code, latitude_deg, longitude_deg, name, municipality }: RawAirport) =>
							/^(small|medium|large)_airport$/.test(type) &&
							iata_code !== '' &&
							data.push({
								name,
								city: municipality,
								id: ident,
								iata: iata_code,
								lat: Number(latitude_deg),
								lng: Number(longitude_deg),
								runways: []
							})
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

		this.cities = this.airports.then((airports) => {
			const groups = new Map<string, Airport[]>();

			airports.forEach((airport) => (groups.has(airport.city) ? groups.get(airport.city)!.push(airport) : groups.set(airport.city, [airport])));

			return Array.from(groups.entries()).map<City>(([name, airports]) => {
				let lat = 0,
					lng = 0;

				airports.forEach((airport) => {
					lat += airport.lat;
					lng += airport.lng;
				});

				lat /= airports.length;
				lng /= airports.length;

				return {
					name,
					lat,
					lng
				};
			});
		});

		this.planes = new Promise<PlaneModel[]>((resolve, reject) => {
			const data: PlaneModel[] = [];

			createReadStream('data/planes.csv')
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
					({
						model,
						mtow,
						to_runway_len,
						land_runway_len,
						c5kft_as,
						cfl150_as,
						cfl240_as,
						cruise_as,
						dfl240_as,
						dfl100_as,
						approach_as
					}: RawPlane) =>
						data.push({
							model,
							mtow: Number(mtow),
							takeoffRunway: Number(to_runway_len),
							landingRunway: Number(land_runway_len),
							climb5kAirspeed: Number(c5kft_as),
							climb15kAirspeed: Number(cfl150_as),
							climb24kAirspeed: Number(cfl240_as),
							cruiseAirspeed: Number(cruise_as),
							desc24kAirspeed: Number(dfl240_as),
							desc10kAirspeed: Number(dfl100_as),
							approachAirspeed: Number(approach_as)
						})
				)
				.on('end', () => resolve(data));
		});
	}
}

