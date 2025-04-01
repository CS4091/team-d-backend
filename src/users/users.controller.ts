import { BadRequestException, Body, Controller, Get, Patch, Post } from '@nestjs/common';
import { ApiResponse } from '@nestjs/swagger';
import type { User } from '@prisma/client';
import { Protected } from 'src/auth/protected.decorator';
import { UsersService } from 'src/users/users.service';
import { ReqUser } from 'src/utils/decorators/user.decorator';
import { LoginDTO, MeUserResponse, PublicUserResponse, RegisterDTO, UpdateNameDTO } from './users.dtos';
import { meUser, MeUser, publicUser } from './users.models';

@Controller('/users')
export class UsersController {
	public constructor(private readonly service: UsersService) {}

	@Get('/')
	@Protected()
	@ApiResponse({ type: PublicUserResponse, isArray: true })
	public async getAllUsers(@ReqUser() user: User): Promise<PublicUserResponse[]> {
		return this.service.getAll({ id: user.id }, publicUser);
	}

	@Get('/me')
	@Protected()
	@ApiResponse({ type: MeUserResponse })
	public async getUser(@ReqUser() user: User): Promise<MeUser> {
		return this.service.get({ id: user.id }, meUser) as Promise<MeUser>;
	}

	@Post('/register')
	@ApiResponse({ type: MeUserResponse })
	public async register(@Body() data: RegisterDTO): Promise<MeUser> {
		return this.service.register(data);
	}

	@Post()
	@ApiResponse({ type: MeUserResponse })
	public async loginUser(@Body() data: LoginDTO): Promise<MeUser> {
		const user = await this.service.login(data);

		if (!user) throw new BadRequestException('No user with specified email/password combination');

		return user;
	}

	@Patch()
	@Protected()
	@ApiResponse({ type: MeUserResponse })
	public async updateName(@ReqUser() user: User, @Body() data: UpdateNameDTO): Promise<MeUser> {
		return this.service.updateName(user, data);
	}
}

