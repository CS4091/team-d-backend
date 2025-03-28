import { DynamicModule, HttpException } from '@nestjs/common';
import { ServeStaticModule } from '@nestjs/serve-static';

export function fi<T>(): T {
	return undefined as T;
}

export function serialize(data: any): string | undefined {
	if (typeof data === 'object') {
		if (data === null) {
			return 'null';
		}

		if (data instanceof Date) {
			return `new Date("${data.toISOString()}")`;
		} else if (Array.isArray(data)) {
			let out = '[';
			for (const elem of data) {
				const serialized = serialize(elem);
				if (serialized !== undefined) {
					out += `${serialized},`;
				}
			}
			if (out !== '[') {
				out = out.slice(0, -1);
			}
			out += ']';

			return out;
		} else {
			let out = '{';
			for (const prop in data) {
				const serialized = serialize(data[prop]);
				if (serialized !== undefined) {
					out += `"${prop}":${serialized},`;
				}
			}
			if (out !== '{') {
				out = out.slice(0, -1);
			}
			out += '}';

			return out;
		}
	} else {
		if (data !== undefined) {
			if (typeof data === 'string') {
				return `"${data.replace('"', '\\"').replace('\\', '\\\\')}"`;
			} else if (typeof data === 'boolean' || typeof data === 'number') {
				return data.toString();
			}
		}
	}
}

export function serveClient(): DynamicModule[] {
	return [
		ServeStaticModule.forRoot({
			rootPath: 'dist/client/assets',
			serveRoot: '/__app'
		}),
		ServeStaticModule.forRoot({
			rootPath: 'src/client/public',
			serveRoot: '/'
		})
	];
}

export function wrap<E extends HttpException, C extends new (...args: any) => E>(ErrClass: C): new (...args: ConstructorParameters<C>) => Error {
	return ErrClass;
}

