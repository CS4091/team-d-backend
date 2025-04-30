import {
	BadRequestException,
	Body,
	Controller,
	Delete,
	Get,
	HttpCode,
	HttpStatus,
	InternalServerErrorException,
	NotFoundException,
	Param,
	Patch,
	Post
} from '@nestjs/common';
import { ApiResponse } from '@nestjs/swagger';
import type { Invite, Organization, User } from '@prisma/client';
import { Protected } from 'src/auth/protected.decorator';
import { UpdateNameDTO } from 'src/users/users.dtos';
import { UsersService } from 'src/users/users.service';
import { ReqUser } from 'src/utils/decorators/user.decorator';
import {
	AcceptInviteDTO,
	CreateInviteDTO,
	CreateOrganizationDTO,
	FullOrganizationResponse,
	InviteResponse,
	OrganizationIDDTO,
	OrganizationInviteDTO,
	OrganizationResponse
} from './orgs.dtos';
import { fullOrg, FullOrganization } from './orgs.models';
import { OrgsService } from './orgs.service';

@Controller('/organizations')
export class OrgsController {
	public constructor(private readonly service: OrgsService, private readonly users: UsersService) {}

	@Get('/:id')
	@Protected()
	@ApiResponse({ type: FullOrganizationResponse })
	public async getOrg(@ReqUser() user: User, @Param() { id }: OrganizationIDDTO): Promise<FullOrganization> {
		return this.service.getOrThrow({ id, users: { some: { id: user.id } } }, fullOrg);
	}

	@Get()
	@Protected()
	@ApiResponse({ type: OrganizationResponse, isArray: true })
	public async getOrgs(@ReqUser() user: User): Promise<Organization[]> {
		return this.service.getAll({ users: { some: { id: user.id } } });
	}

	@Post()
	@Protected()
	@ApiResponse({ type: OrganizationResponse })
	public async createOrg(@ReqUser() user: User, @Body() data: CreateOrganizationDTO): Promise<Organization> {
		return this.service.create(user, data);
	}

	@Post('/:id/invite')
	@Protected()
	@ApiResponse({ type: InviteResponse })
	public async inviteUser(@ReqUser() user: User, @Param() orgId: OrganizationIDDTO, @Body() { userId }: CreateInviteDTO): Promise<Invite> {
		const org = await this.service.get(orgId, fullOrg);

		if (!org || !org.users.some((u) => u.id === user.id)) throw new NotFoundException(`Organization with id '${orgId.id}' does not exist.`);

		const other = await this.users.get({ id: userId });

		if (!other) throw new BadRequestException(`User with id '${userId}' does not exist.`);

		try {
			return await this.service.invite(org, other);
		} catch (err: unknown) {
			if (err instanceof OrgsService.DuplicateException) {
				throw new BadRequestException(`User with id '${userId}' is already invited to organization '${org.name}'.`);
			} else {
				throw new InternalServerErrorException('An unknown error occured.');
			}
		}
	}

	@Delete('/:id/invite/:token')
	@Protected()
	@ApiResponse({ type: FullOrganizationResponse })
	@HttpCode(HttpStatus.NO_CONTENT)
	public async cancelInvite(@ReqUser() user: User, @Param() inviteId: OrganizationInviteDTO): Promise<FullOrganization> {
		const org = await this.service.get({ id: inviteId.id }, fullOrg);

		if (!org || !org.users.some((u) => u.id === user.id)) throw new NotFoundException(`Organization with id '${inviteId.id}' does not exist.`);

		return this.service.cancelInvite(org, inviteId.token);
	}

	@Post('/accept-invite')
	@Protected()
	@ApiResponse({ type: FullOrganizationResponse })
	public async acceptInvite(@ReqUser() user: User, @Body() { token }: AcceptInviteDTO): Promise<FullOrganization> {
		try {
			return await this.service.acceptInvite(user, token);
		} catch (err: unknown) {
			if (err instanceof OrgsService.NotAllowedException) {
				throw new NotFoundException(`Invite with token '${token}' does not exist.`);
			} else if (err instanceof OrgsService.InviteExpiredException) {
				throw new BadRequestException('Invite is expired.');
			} else {
				throw new InternalServerErrorException('An unknown error occured.');
			}
		}
	}

	@Post('/decline-invite')
	@Protected()
	@HttpCode(HttpStatus.NO_CONTENT)
	public async declineInvite(@ReqUser() user: User, @Body() { token }: AcceptInviteDTO): Promise<void> {
		return this.service.declineInvite(user, token);
	}

	@Patch('/:id')
	@Protected()
	@ApiResponse({ type: OrganizationResponse })
	public async updateOrgName(@ReqUser() user: User, @Param() orgId: OrganizationIDDTO, @Body() { name }: UpdateNameDTO): Promise<Organization> {
		const org = await this.service.get(orgId, fullOrg);

		if (!org || !org.users.some((u) => u.id === user.id)) throw new NotFoundException(`Organization with id '${orgId.id}' does not exist.`);

		return await this.service.updateOrgName(org, name);
	}
}

