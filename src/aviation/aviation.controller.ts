import { BadRequestException, Body, Controller, Get, Post } from '@nestjs/common';
import { ApiResponse } from '@nestjs/swagger';
import type { User } from '@prisma/client';
import { Protected } from 'src/auth/protected.decorator';
import { fullOrg } from 'src/orgs/orgs.models';
import { OrgsService } from 'src/orgs/orgs.service';
import { ReqUser } from 'src/utils/decorators/user.decorator';
import { RouteResponse, RouteResult, RouteScenario } from './aviation.dtos';
import { Airport, City, PlaneModel } from './aviation.models';
import { AviationService } from './aviation.service';
import { RoutingService } from './routing.service';

@Controller('/aviation')
export class AviationController {
	public constructor(public readonly service: AviationService, private readonly routing: RoutingService, private readonly orgs: OrgsService) {}

	@Get('/airports')
	@ApiResponse({ type: Airport, isArray: true })
	public async getAirports(): Promise<Airport[]> {
		return this.service.airports;
	}

	@Get('/cities')
	@ApiResponse({ type: City, isArray: true })
	public async getCities(): Promise<City[]> {
		return this.service.cities;
	}

	@Get('/planes')
	@ApiResponse({ type: PlaneModel, isArray: true })
	public async getPlanes(): Promise<PlaneModel[]> {
		return this.service.planes;
	}

	@Post('/route')
	@ApiResponse({ type: RouteResponse })
	@Protected()
	public async planRoute(@Body() { organizationId, demand }: RouteScenario, @ReqUser() user: User): Promise<RouteResponse> {
		const org = await this.orgs.get({ id: organizationId }, fullOrg);

		if (!org || !org.users.some((u) => user.id === u.id)) throw new BadRequestException('Unknown organization.');

		const cities = await this.service.cities,
			airports = await this.service.airports;
		if (
			!demand.every(
				({ from, to, passengers }) =>
					passengers > 0 &&
					from !== to &&
					Number.isInteger(passengers) &&
					airports.some((airport) => airport.id === from) &&
					airports.some((airport) => airport.id === to)
			)
		)
			throw new BadRequestException('Demand must be an array of object requests with req.from != req.to, each in the list of available airports.');

		const planeModels = await this.service.planes;
		const speccedPlanes = org.planes.map((plane) => ({ ...plane, specs: planeModels.find((p) => p.model === plane.model) }));

		const missingPlane = speccedPlanes.find((plane) => !plane.specs);
		if (missingPlane) throw new BadRequestException(`Unknown plane model '${missingPlane.model}'.`);

		return this.routing.route(cities, await this.service.airports, demand, speccedPlanes as any);
	}

	@Post('/prep')
	@ApiResponse({ type: RouteResult })
	public async prepEnv(): Promise<void> {
		const cities = await this.service.cities;
		const planeModels = await this.service.planes;
		const airports = await this.service.airports;

		return this.routing.prep(cities, planeModels);
	}
}

