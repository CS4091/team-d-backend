import { writeFileSync } from 'fs';
import { JSDOM } from 'jsdom';

Promise.all(
	['B733', 'B741', 'A320', 'A332'].map((plane) =>
		fetch(`https://contentzone.eurocontrol.int/aircraftperformance/details.aspx?ICAO=${plane}`)
			.then((res) => res.text())
			.then((html) => new JSDOM(html))
			.then(({ window: { document } }) => {
				const data = { plane };

				Array.from(document.querySelectorAll('h4 + div.row')).map((node) => {
					const phase = (data[node.previousElementSibling.textContent] = {});

					Array.from(node.querySelectorAll('div p')).forEach((block) => {
						const [stat, val] = Array.from(block.querySelectorAll('span')).map((part) => part.textContent);
						const value = Number(val);

						if (!Number.isNaN(value)) phase[stat] = value;
					});
				});

				return data;
			})
	)
).then((planes) => {
	let csv = 'model,mtow,to_runway_len,land_runway_len,range,c5kft_as,cfl150_as,cfl240_as,cruise_as,dfl240_as,dfl100_as,approach_as\n';

	planes.forEach(
		({
			plane: model,
			'Take-off': takeoff,
			Landing: land,
			'Initial climb (to 5000ft)': c5kft,
			'Climb (to FL 150)': cfl150,
			'Climb (to FL 240)': cfl240,
			Cruise: cruise,
			'Initial Descent (to FL 240)': dfl240,
			'Descent (to FL 100)': dfl100,
			Approach: approach
		}) =>
			(csv +=
				[
					model,
					takeoff.MTOW,
					takeoff.Distance,
					land.Distance,
					nmToMeters(cruise.Range),
					nmToMeters(c5kft.IAS),
					nmToMeters(cfl150.IAS),
					nmToMeters(cfl240.IAS),
					nmToMeters(cruise.TAS),
					nmToMeters(dfl240.MACH * 602),
					nmToMeters(dfl100.IAS),
					nmToMeters(approach.IAS)
				]
					.map((val) => (typeof val === 'number' ? (Number.isInteger(val) ? val : val.toFixed(4)) : val))
					.join(',') + '\n')
	);

	writeFileSync('data/planes.csv', csv);
});

function nmToMeters(kts) {
	return (kts / 1.08 / 1.6) * 1000;
}

