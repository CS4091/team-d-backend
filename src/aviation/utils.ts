export function processHeaders(headers: (string | null | undefined)[]): string[] {
	return headers.map((header) => {
		if (typeof header !== 'string') throw new Error('Bad airports.csv header');

		const [first, ...rest] = header.split(/,?\s+/);

		return [
			first.toLowerCase(),
			...rest.map((part) =>
				part.toLowerCase() === 'id' ? part.toUpperCase() : part.toUpperCase() === part ? part : part[0].toUpperCase() + part.slice(1).toLowerCase()
			)
		].join('');
	});
}

