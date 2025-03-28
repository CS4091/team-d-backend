import { parse } from '@fast-csv/parse';
import { createReadStream, writeFileSync } from 'fs';
import { JSDOM } from 'jsdom';

new Promise((resolve, reject) => {
	const data = [];

	new Promise((resolve, reject) =>
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
				({ type, ident, iata_code, iso_country }) =>
					/^(small|medium|large)_airport$/.test(type) &&
					iata_code !== '' &&
					data.push({
						country: iso_country,
						id: ident,
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
				.on('data', ({ airport_ident, le_ident }) => data.find((airport) => airport.id === airport_ident)?.runways.push({ name: le_ident }))
				.on('end', () => resolve(data))
		)
		.catch((err) => reject(err));
})
	.then((airports) => airports.filter((airport) => airport.runways.length > 0))
	.then((airports) =>
		batch(airports, ({ id, country }) =>
			country !== 'US'
				? { id, price: 8 }
				: fetch(`http://www.100ll.com/searchresults.php?searchfor=${id}`).then((res) => {
						if (res.status === 302) {
							return fetch(`http://www.100ll.com/${res.headers.get('Location')}`)
								.then((res) => res.text())
								.then((html) => new JSDOM(html))
								.then(scrapePrice(id));
						} else {
							return res
								.text()
								.then((html) => new JSDOM(html))
								.then(({ window: { document } }) => {
									const suppliers = Array.from(
										document.body.querySelectorAll('table.Content-dl td:has(img[alt="fuel"]) ~ td a:not(:has(img))')
									).map((elem) => `http://www.100ll.com/${elem.href}`);

									if (suppliers.length === 0) return { id, price: 8 };

									return Promise.all(
										suppliers.map((url) =>
											fetch(url)
												.then((res) => res.text())
												.then((html) => new JSDOM(html))
												.then(scrapePrice(id))
										)
									).then((prices) => prices.slice(1).reduce((curr, offer) => (offer.price < offer.price ? offer : curr), prices[0]));
								});
						}
				  })
		)
	)
	.then((prices) => {
		let csv = 'id,price\n';

		prices.forEach(
			({ id, price }) =>
				(csv += [id, price].map((val) => (typeof val === 'number' ? (Number.isInteger(val) ? val : val.toFixed(4)) : val)).join(',') + '\n')
		);

		writeFileSync('data/fuel.csv', csv);
	});

function scrapePrice(id) {
	return ({ window: { document } }) => {
		const fuelNames = Array.from(document.body.querySelectorAll('table.Content-dl table.Content-dl tr:has(td[colspan="2"]):has(+tr)')).filter(
			(elem) => elem.nextElementSibling.querySelector('td+td b') !== null
		);

		const desiredElem = fuelNames.find((elem) => elem.textContent.trim() === '100LL');
		if (!desiredElem) {
			console.error('No 100LL on airport', id);
			return { id, price: Infinity };
		}
		const rawData = desiredElem.nextElementSibling.textContent.trim();

		const match = rawData.match(/^.+: \$(.*)\/Gal$/);
		if (!match) {
			console.error('No value on airport', id, rawData);
			return { id, price: Infinity };
		}
		const price = Number(match[1]);

		return { id, price };
	};
}

function batch(data, mapper) {
	return new Promise((resolve, reject) => {
		const batch = data.slice(0, 10);

		Promise.all(batch.map(mapper)).then(resolve).catch(reject);
	}).then((results) => (data.length > 10 ? batch(data.slice(10), mapper).then((next) => results.concat(next)) : results));
}

