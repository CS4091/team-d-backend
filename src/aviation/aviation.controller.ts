import { Controller, Get } from '@nestjs/common';
import { ApiResponse } from '@nestjs/swagger';
import { Airport, PlaneModel } from './aviation.models';
import { AviationService } from './aviation.service';

@Controller('/aviation')
export class AviationController {
	public constructor(public readonly service: AviationService) {}

	@Get('/airports')
	@ApiResponse({ type: Airport, isArray: true })
	public async getAirports(): Promise<Airport[]> {
		return this.service.airports;
	}

	@Get('/planes')
	@ApiResponse({ type: PlaneModel, isArray: true })
	public async getPlanes(): Promise<PlaneModel[]> {
		return this.service.planes;
	}
}

