import { BadRequestException, Body, Controller, Delete, Get, NotFoundException, Param, Patch, Post } from '@nestjs/common';
import { ApiResponse } from '@nestjs/swagger';
import type { Plane, User } from '@prisma/client';
import { Protected } from 'src/auth/protected.decorator';
import { AviationService } from 'src/aviation/aviation.service';
import { OrganizationIDDTO, OrganizationPlaneIDDTO } from 'src/orgs/orgs.dtos';
import { fullOrg } from 'src/orgs/orgs.models';
import { OrgsService } from 'src/orgs/orgs.service';
import { ReqUser } from 'src/utils/decorators/user.decorator';
import { CreatePlaneDTO, PlaneResponse, UpdatePlaneDTO } from './assets.dtos';
import { AssetsService } from './assets.service';

@Controller('/organizations/:id/assets')
export class AssetsController {
	public constructor(
		public readonly aviationService: AviationService,
		private readonly service: AssetsService,
		private readonly organizations: OrgsService
	) {}

	@Post()
	@Protected()
	@ApiResponse({ type: PlaneResponse })
	public async addPlane(@ReqUser() user: User, @Param() { id }: OrganizationIDDTO, @Body() data: CreatePlaneDTO): Promise<Plane> {
		const org = await this.organizations.get({ id }, fullOrg);

		if (!org || !org.users.some((u) => u.id === user.id)) throw new NotFoundException(`Organization with id '${id}' does not exist.`);

		const airports = await this.aviationService.airports;
		if (!airports.some((airport) => airport.id === data.homeBase))
			throw new BadRequestException('Plane home base must be from the list of available airports.');

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

	@Delete('/:planeId')
	@Protected()
	@ApiResponse({ type: PlaneResponse })
	public async deletePlane(@ReqUser() user: User, @Param() { id, planeId }: OrganizationPlaneIDDTO) {
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

		const airports = await this.aviationService.airports;
		if (!airports.some((airport) => airport.id === updates.homeBase))
			throw new BadRequestException('Plane home base must be from the list of available cities.');

		return await this.service.update(updates, org, planeId);
	}
}

