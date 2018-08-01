export interface ITPHReading {
    id: string;
    humidity: number;
    pressure: number;
    temperature: number;
    readAt: Date;
}

export interface IReadingAggregate {
    minimum: number;
    maximum: number;
    total: number;
    count: number;
}

export interface IAggregateRecord {
    id: string;
    timeRange: string;
    temperature: IReadingAggregate;
    pressure: IReadingAggregate;
    humidity: IReadingAggregate;
}

export interface IDevice {
    account: string;
    deviceId: string;
    active: boolean;
    description: string;
    location: string;
}
