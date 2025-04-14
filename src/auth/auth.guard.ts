import { CanActivate, ExecutionContext, Injectable } from '@nestjs/common';
import { Reflector } from '@nestjs/core';
import { Request } from 'express';
import { Redirect } from 'src/utils/filters/redirect.filter';
import { _Protected } from './protected.decorator';

@Injectable()
export class AuthGuard implements CanActivate {
	public constructor(private readonly reflector: Reflector) {}

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

