import { Controller, Get } from '@nestjs/common';
import { ApiResponse } from '@nestjs/swagger';
import { Airport } from './aviation.models';
import { AviationService } from './aviation.service';

@Controller('/aviation')
export class AviationController {
	public constructor(public readonly service: AviationService) {}

	@Get('/airports')
	@ApiResponse({ type: Airport })
	public async getAirports(): Promise<Airport[]> {
		return this.service.airports;
	}
}

