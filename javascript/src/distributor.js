class Distributor {
    constructor() {
        this._clients = [];
        this._channel_to_sinks = new Map();
    }

    addClient(client) {
        this._clients.push(client);
    }

    onOpen() {
        this._clients.forEach((client) => client.onOpen());
    }

    onClose() {
        this._clients.forEach((client) => client.onClose());
    }

    onEvent(event) {
        if (event.hasHello()) {
            let hello = event.getHello();
            hello.getChannelsList().forEach((channel) => this.onChannel(channel));
            return;
        }

        if (!this._channel_to_sinks.has(event.getChannel())) return;
        const sinks = this._channel_to_sinks.get(event.getChannel());
        sinks.forEach((sink) => sink(event));
    }

    onChannel(channel) {
        let sinks = [];
        this._channel_to_sinks.set(channel.getChannel(), sinks);

        this._clients.forEach((client) => {
            let sink = client.onChannel(channel);
            if (sink) sinks.push(sink);
        });
    }
}

export { Distributor };
