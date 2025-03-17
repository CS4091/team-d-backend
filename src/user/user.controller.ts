import { Controller, Get, Post } from '@nestjs/common';
import { ApiResponse } from '@nestjs/swagger';
import { Protected } from 'src/auth/protected.decorator';
import { Prisma, User } from '@prisma/client';
import { UserService } from 'src/user/user.service';
import { ReqUser } from 'src/utils/decorators/user.decorator';
import { PublicUserResponse,  RegisterDTO, LoginDTO } from './user.dtos';
import { MeUser, meUser } from './user.models';

@Controller('/user')
export class UserController {
  public constructor(private readonly service: UserService) {}

  @Get()
  @Protected()
  @ApiResponse({ type: PublicUserResponse})
  public async getUser<S extends Prisma.UserDefaultArgs>(
    where: Prisma.UserWhereUniqueInput,
    selectors: S = {} as any
  ):Promise<Prisma.UserGetPayload<S> | null>{
    return this.service.get(where, selectors) as any;
  }

  @Post()
  @Protected()
  @ApiResponse({ type: PublicUserResponse })
  public async registerUser(data: RegisterDTO): Promise<MeUser> {
    return this.service.register(data);
  }

  @Post()
  @Protected()
  @ApiResponse({ type: PublicUserResponse})
  public async loginUser(data: LoginDTO): Promise<MeUser | null> {
    return this.service.login(data);
  }

  @Post()
  @Protected()
  @ApiResponse({ type: PublicUserResponse})
  public async authUser(token: string): Promise<User | null> {
    return this.service.auth(token);
  }
}