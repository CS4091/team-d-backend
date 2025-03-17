import { BadRequestException, Controller, Get, InternalServerErrorException, NotFoundException, Param, Post, Query } from '@nestjs/common';
import { ApiResponse } from '@nestjs/swagger';
import type { Invite, Organization, User } from '@prisma/client';
import { Protected } from 'src/auth/protected.decorator';
import { UserService } from 'src/user/user.service';
import { ReqUser } from 'src/utils/decorators/user.decorator';
import { AcceptInviteDTO, CreateInviteDTO, CreateOrganizationDTO, InviteResponse, OrganizationIDDTO, OrganizationResponse } from './org.dtos';
import { fullOrg } from './org.models';
import { OrgService } from './org.service';

@Controller('/organizations')
export class OrgController {
	public constructor(private readonly service: OrgService, private readonly users: UserService) {}

	@Get()
	@Protected()
	@ApiResponse({ type: OrganizationResponse, isArray: true })
	public async getOrgs(@ReqUser() user: User): Promise<Organization[]> {
		return this.service.getAll({ users: { some: { id: user.id } } });
	}

	@Post()
	@Protected()
	@ApiResponse({ type: OrganizationResponse })
	public async createOrg(@ReqUser() user: User, data: CreateOrganizationDTO): Promise<Organization> {
		return this.service.create(user, data);
	}

	@Post('/:id/invite')
	@Protected()
	@ApiResponse({ type: InviteResponse })
	public async inviteUser(@ReqUser() user: User, @Param() orgId: OrganizationIDDTO, @Query() { userId }: CreateInviteDTO): Promise<Invite> {
		const org = await this.service.get(orgId, fullOrg);

		if (!org || !org.users.some((u) => u.id === user.id)) throw new NotFoundException(`Organization with id '${orgId.id}' does not exist.`);

		const other = await this.users.get({ id: userId });

		if (!other) throw new BadRequestException(`User with id '${userId}' does not exist.`);

		try {
			return await this.service.invite(org, other);
		} catch (err: unknown) {
			if (err instanceof OrgService.DuplicateException) {
				throw new BadRequestException(`User with id '${userId}' is already invited to organization '${org.name}'.`);
			} else {
				throw new InternalServerErrorException('An unknown error occured.');
			}
		}
	}

	@Post('/accept-invite')
	@Protected()
	@ApiResponse({ type: OrganizationResponse })
	public async acceptInvite(@ReqUser() user: User, @Query() { token }: AcceptInviteDTO): Promise<Organization> {
		try {
			return await this.service.acceptInvite(user, token);
		} catch (err: unknown) {
			if (err instanceof OrgService.NotAllowedException) {
				throw new NotFoundException(`Invite with token '${token}' does not exist.`);
			} else if (err instanceof OrgService.InviteExpiredException) {
				throw new BadRequestException('Invite is expired.');
			} else {
				throw new InternalServerErrorException('An unknown error occured.');
			}
		}
	}
}

