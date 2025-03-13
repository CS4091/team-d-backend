import { Module } from '@nestjs/common';
import { DBModule } from 'src/db/db.module';
import { AviationController } from './aviation.controller';
import { AviationService } from './aviation.service';

@Module({
	imports: [DBModule],
	providers: [AviationService],
	exports: [AviationService],
	controllers: [AviationController]
})
export class AviationModule {}

