import { Body, Controller, Delete, NotFoundException, Param, Patch, Post } from '@nestjs/common';
import { ApiResponse } from '@nestjs/swagger';
import type { Plane, User } from '@prisma/client';
import { Protected } from 'src/auth/protected.decorator';
import { OrganizationIDDTO } from 'src/orgs/orgs.dtos';
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

	@Delete(':planeId')
	@Protected()
	@ApiResponse({ type: PlaneResponse })
	public async deletePlane(
		@ReqUser() user: User,
		@Param() { id }: OrganizationIDDTO,
		@Param('planeId') planeId: string
	): Promise<Plane> {
		const org = await this.organizations.get({ id }, fullOrg);

		if (!org || !org.users.some((u) => u.id === user.id)) {
			throw new NotFoundException(`Organization with id '${id}' does not exist.`);
		}

		const match = org.planes.find((p) => p.id === planeId);
		if (!match) {
			throw new NotFoundException(`Plane with id '${planeId}' not found in this organization.`);
		}

		return this.service.delete(planeId, org.id);
	}

	@Patch(':planeId')
	@Protected()
	@ApiResponse({ type: PlaneResponse })
	public async updatePlane(
		@ReqUser() user: User,
		@Param() { id }: OrganizationIDDTO,
		@Param('planeId') planeId: string,
		@Body() data: UpdatePlaneDTO
	): Promise<Plane> {
		const org = await this.organizations.get({ id }, fullOrg);

		if (!org || !org.users.some((u) => u.id === user.id)) {
			throw new NotFoundException(`Organization with id '${id}' does not exist.`);
		}

		const plane = org.planes.find((p) => p.id === planeId);
		if (!plane) {
			throw new NotFoundException(`Plane with id '${planeId}' not found in this organization.`);
		}

		return this.service.update(planeId, id, data);
	}
}

