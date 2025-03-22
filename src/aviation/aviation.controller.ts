import { Controller, Get } from '@nestjs/common';
import { ApiResponse } from '@nestjs/swagger';
import { City, PlaneModel } from './aviation.models';
import { AviationService } from './aviation.service';

@Controller('/aviation')
export class AviationController {
	public constructor(public readonly service: AviationService) {}

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
}

