import { Inject, Injectable, NestMiddleware, forwardRef } from '@nestjs/common';
import { NextFunction, Request } from 'express';
import { AuthDataSource, DATA_SOURCE, PREFIX } from './auth.module';

@Injectable()
export class AuthMiddleware implements NestMiddleware {
	public constructor(
		@Inject(forwardRef(() => DATA_SOURCE)) private readonly source: AuthDataSource,
		@Inject(forwardRef(() => PREFIX)) private readonly prefix: string
	) {}

	use(req: Request, _, next: NextFunction) {
		const token = req.header('Authorization')?.split(' ')[1] ?? req.body.token ?? req.query.token;

		if (token !== undefined) {
			req.token = token;
			const user = this.source.auth(token);

			if (user === null) {
				req.user = user;
				next();
			} else {
				if ('then' in user) {
					user.then((user: any) => {
						req.user = user;
						next();
					});
				} else {
					req.user = user;
					next();
				}
			}
		} else {
			req.token = null;
			req.user = null;
			next();
		}
	}
}

