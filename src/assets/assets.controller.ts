import { Body, Controller, Get, NotFoundException, Param, Patch, Post, Delete } from '@nestjs/common';
import { ApiResponse } from '@nestjs/swagger';
import type { Plane, User } from '@prisma/client';
import { Protected } from 'src/auth/protected.decorator';
import { OrganizationIDDTO, OrganizationPlaneIDDTO } from 'src/orgs/orgs.dtos';
import { fullOrg } from 'src/orgs/orgs.models';
import { OrgsService } from 'src/orgs/orgs.service';
import { ReqUser } from 'src/utils/decorators/user.decorator';
import { CreatePlaneDTO, PlaneResponse, UpdatePlaneDTO } from './assets.dtos';
import { AssetsService } from './assets.service';

@Controller('/organizations/:id/assets')
export class AssetsController {
	public constructor(private readonly service: AssetsService, private readonly organizations: OrgsService) {}

	@Post()
	@Protected()
	@ApiResponse({ type: PlaneResponse })
	public async addPlane(@ReqUser() user: User, @Param() { id }: OrganizationIDDTO, @Body() data: CreatePlaneDTO): Promise<Plane> {
		const org = await this.organizations.get({ id }, fullOrg);

		if (!org || !org.users.some((u) => u.id === user.id)) throw new NotFoundException(`Organization with id '${id}' does not exist.`);

		return this.service.create(data, org);
	}

	@Get()
	@Protected()
	@ApiResponse({ type: PlaneResponse, isArray: true })
	public async getPlanes(@ReqUser() user: User, @Param() { id }: OrganizationIDDTO): Promise<Plane[]> {
		const org = await this.organizations.get({ id }, fullOrg);

		if (!org || !org.users.some((u) => u.id === user.id)) throw new NotFoundException(`Organization with id '${id}' does not exist.`);

		return org.planes;
	}

	@Delete()
	@Protected()
	@ApiResponse({ type: PlaneResponse })
	public async deletePlane(@ReqUser() user: User, @Param() { id, planeId }: OrganizationPlaneIDDTO)  {
	     const org = await this.organizations.get({ id }, fullOrg);

		if (!org || !org.users.some((u) => u.id === user.id)) throw new NotFoundException(`Organization with id '${id}' does not exist.`);

	     return this.service.delete(org, planeId);
	}

	@Patch('/:planeId')
	@Protected()
	@ApiResponse({ type: PlaneResponse })
	public async updatePlane(@ReqUser() user: User, @Param() { id, planeId }: OrganizationPlaneIDDTO, @Body() updates: UpdatePlaneDTO): Promise<Plane> {
		const org = await this.organizations.get({ id }, fullOrg);

		if (!org || !org.users.some((u) => u.id === user.id)) throw new NotFoundException(`Organization with id '${id}' does not exist.`);

		return await this.service.update(updates, org, planeId);
	}
}

