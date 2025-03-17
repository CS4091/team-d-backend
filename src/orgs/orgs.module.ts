import { Module } from '@nestjs/common';
import { DBModule } from 'src/db/db.module';
import { UsersModule } from 'src/users/users.module';
import { OrgsController } from './orgs.controller';
import { OrgsService } from './orgs.service';

@Module({
	imports: [DBModule, UsersModule],
	providers: [OrgsService],
	controllers: [OrgsController],
	exports: [OrgsService]
})
export class OrgsModule {}

