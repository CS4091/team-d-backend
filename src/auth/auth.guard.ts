import { CanActivate, ExecutionContext, forwardRef, Inject, Injectable } from '@nestjs/common';
import { Reflector } from '@nestjs/core';
import { Request } from 'express';
import { Redirect } from 'src/utils/filters/redirect.filter';
import { PREFIX } from './auth.module';
import { _Protected } from './protected.decorator';

@Injectable()
export class AuthGuard implements CanActivate {
	public constructor(@Inject(forwardRef(() => PREFIX)) private readonly prefix: string, private readonly reflector: Reflector) {}

	public async canActivate(context: ExecutionContext): Promise<boolean> {
		const metadata = this.reflector.get(_Protected, context.getHandler());

		if (metadata) {
			const req = context.switchToHttp().getRequest<Request>();
			const user = req.user;

			if (user === null) {
				throw new Redirect('/login');
			} else {
				return false;
			}
		} else {
			return true;
		}
	}
}

