import { BadRequestException, Body, Controller, Get, Post } from '@nestjs/common';
import { ApiResponse } from '@nestjs/swagger';
import type { User } from '@prisma/client';
import { Protected } from 'src/auth/protected.decorator';
import { fullOrg } from 'src/orgs/orgs.models';
import { OrgsService } from 'src/orgs/orgs.service';
import { ReqUser } from 'src/utils/decorators/user.decorator';
import { RouteResult, RouteScenario } from './aviation.dtos';
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
	@ApiResponse({ type: RouteResult })
	@Protected()
	public async planRoute(@Body() { organizationId, demand }: RouteScenario, @ReqUser() user: User): Promise<RouteResult> {
		const org = await this.orgs.get({ id: organizationId }, fullOrg);

		if (!org || !org.users.some((u) => user.id === u.id)) throw new BadRequestException('Unknown organization.');

		const cities = await this.service.cities,
			airports = await this.service.airports;
		if (
			!demand.every(
				(pair) =>
					pair.length === 2 &&
					pair[0] !== pair[1] &&
					(cities.some((city) => city.name === pair[0]) || airports.some((airport) => airport.id === pair[0])) &&
					(cities.some((city) => city.name === pair[1]) || airports.some((airport) => airport.id === pair[1]))
			)
		)
			throw new BadRequestException('Demand must be an array of [from, to] tuples with from != to, each in the list of available cities.');

		const planeModels = await this.service.planes;
		const speccedPlanes = org.planes.map((plane) => ({ ...plane, specs: planeModels.find((p) => p.model === plane.model) }));

		const missingPlane = speccedPlanes.find((plane) => !plane.specs);
		if (missingPlane) throw new BadRequestException(`Unknown plane model '${missingPlane.model}'.`);

		return this.routing.route(cities, demand, speccedPlanes as any);
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

