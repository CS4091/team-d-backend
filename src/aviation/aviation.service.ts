import { parse } from '@fast-csv/parse';
import { Injectable } from '@nestjs/common';
import { createReadStream } from 'fs';
import { Airport, RawAirport } from './aviation.models';

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
						headers: (headers: (string | null | undefined)[]) =>
							headers.map((header) => {
								if (typeof header !== 'string') throw new Error('Bad airports.csv header');

								const [first, ...rest] = header.split(/,?\s+/);

								return [
									first.toLowerCase(),
									...rest.map((part) =>
										part.toLowerCase() === 'id'
											? part.toUpperCase()
											: part.toUpperCase() === part
											? part
											: part[0].toUpperCase() + part.slice(1).toLowerCase()
									)
								].join('');
							})
					})
				)
				.on('error', reject)
				.on(
					'data',
					({ facilityType, name, icaoID, arpLatitudeDD, arpLongitudeDD }: RawAirport) =>
						facilityType === 'AIRPORT' && icaoID !== '' && data.push({ name, id: icaoID, lat: Number(arpLatitudeDD), lng: Number(arpLongitudeDD) })
				)
				.on('end', () => resolve(data));
		}).catch((err) => {
			console.error(err);
			throw new Error('Failed to acquire airports.csv');
		});
	}
}

