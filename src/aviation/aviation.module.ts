import { Module } from '@nestjs/common';
import { DBModule } from 'src/db/db.module';
import { AviationService } from './aviation.service';

@Module({
	imports: [DBModule],
	providers: [AviationService],
	exports: [AviationService]
})
export class AviationModule {}

