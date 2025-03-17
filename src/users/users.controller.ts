import { BadRequestException, Controller, Get, Post } from '@nestjs/common';
import { ApiResponse } from '@nestjs/swagger';
import type { User } from '@prisma/client';
import { Protected } from 'src/auth/protected.decorator';
import { UsersService } from 'src/users/users.service';
import { ReqUser } from 'src/utils/decorators/user.decorator';
import { LoginDTO, MeUserResponse, RegisterDTO } from './users.dtos';
import { meUser, MeUser } from './users.models';

@Controller('/users')
export class UsersController {
	public constructor(private readonly service: UsersService) {}

	@Get('/me')
	@Protected()
	@ApiResponse({ type: MeUserResponse })
	public async getUser(@ReqUser() user: User): Promise<MeUser> {
		return this.service.get({ id: user.id }, meUser) as Promise<MeUser>;
	}

	@Post('/register')
	@ApiResponse({ type: MeUserResponse })
	public async register(data: RegisterDTO): Promise<MeUser> {
		return this.service.register(data);
	}

	@Post()
	@ApiResponse({ type: MeUserResponse })
	public async loginUser(data: LoginDTO): Promise<MeUser> {
		const user = await this.service.login(data);

		if (!user) throw new BadRequestException('No user with specified email/password combination');

		return user;
	}
}

