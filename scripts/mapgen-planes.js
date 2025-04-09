import { parse } from '@fast-csv/parse';
import { createReadStream, writeFileSync } from 'fs';

const csvPath = process.argv[2];

new Promise((resolve, reject) => {
	const planes = [];

	createReadStream(csvPath)
		.pipe(parse({ objectMode: true, trim: true, headers: true }))
		.on('data', (data) => planes.push(data))
		.on('end', () => resolve(planes))
		.on('error', (err) => reject(err));
}).then((planes) => {
	writeFileSync(
		'planes.json',
		JSON.stringify(
			planes.map(
				({
					model,
					to_runway_len: takeoffRunway,
					land_runway_len: landingRunway,
					range,
					c5kft_as: climb5kAirspeed,
					cfl150_as: climb15kAirspeed,
					cfl240_as: climb24kAirspeed,
					cruise_as: cruiseAirspeed,
					dfl240_as: desc24kAirspeed,
					dfl100_as: desc10kAirspeed,
					approach_as: approachAirspeed
				}) => ({
					model,
					range: Number(range),
					takeoffRunway: Number(takeoffRunway),
					landingRunway: Number(landingRunway),
					climb5kAirspeed: Number(climb5kAirspeed),
					climb15kAirspeed: Number(climb15kAirspeed),
					climb24kAirspeed: Number(climb24kAirspeed),
					cruiseAirspeed: Number(cruiseAirspeed),
					desc24kAirspeed: Number(desc24kAirspeed),
					desc10kAirspeed: Number(desc10kAirspeed),
					approachAirspeed: Number(approachAirspeed)
				})
			)
		)
	);
});

