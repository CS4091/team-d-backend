import { Module } from '@nestjs/common';
import { DBModule } from 'src/db/db.module';
import { OrgsModule } from 'src/orgs/orgs.module';
import { AviationController } from './aviation.controller';
import { AviationService } from './aviation.service';
import { RoutingService } from './routing.service';

@Module({
	imports: [DBModule, OrgsModule],
	providers: [AviationService, RoutingService],
	exports: [AviationService],
	controllers: [AviationController]
})
export class AviationModule {}

